import aiocoap
import asyncio
import logging
import json
import time
import math
from typing import Dict, List
import calendar

from datetime import datetime
from influxdb_client.client.influxdb_client_async import InfluxDBClientAsync, WriteApiAsync
from influxdb_client import Point
logging.basicConfig(level=logging.INFO)

# if this fails, refer to template_secret.py
from secret import *

aiocoap.numbers.constants.MAX_RETRANSMIT = 0
aiocoap.numbers.constants.ACK_TIMEOUT = 1
aiocoap.numbers.constants.NSTART = 10

devices = None
basestation_lines = []

# enable block transfers
use_block_transfers = True

# This map contains shortened names for some of the value names from the smartwatch.
# This makes it possible to always display these values on the basestation without horizontal scrolling.
rename_map = {
    "sweating": "sweating",
    "x_acceleration_m/s^2": "x_acc",
    "y_acceleration_m/s^2": "y_acc",
    "z_acceleration_m/s^2": "z_acc",
    "a_rotation_deg/s": "a_rot",
    "b_rotation_deg/s": "b_rot",
    "c_rotation_deg/s": "c_rot",
    "temperature_degC": "temp",
    "display_number": "time",
    "value": "water"
}

integer_vars = ["display_number"]

# Fetch all devices(=components, sensors) from the smartwatch.
# This makes it possible to add sensor/components without changing the python code.
async def fetch_devices(protocol):
    uri = smartwatch_url + "/device"
    req = aiocoap.Message(code=aiocoap.Code.GET, uri=uri)
    try:
        resp : aiocoap.Message = await protocol.request(req).response
        respJson = json.loads(resp.payload)
        if respJson["type"] != "ok":
            print("Reading sensor: Remote error:", respJson["msg"])
        else:
            return respJson["devices"]
    except:
        print("No esp-smartwatch found")
    
    return None

# Sets the time on the smartwatch to the time specified by timestamp.
# Timestamp should be adjusted to the timezone.
async def write_to_smartwatch_rtc(protocol, timestamp: int):
    print(timestamp)
    req = aiocoap.Message(
        code=aiocoap.Code.POST, 
        uri=smartwatch_url + "/device/rtc-display-1/command",
        payload=json.dumps({"command": "set_unix_time", "unixtime": timestamp}).encode("utf-8"))
    try:
        print("Setting esp-smartwatch time")
        resp : aiocoap.Message = await protocol.request(req).response
        respJson = json.loads(resp.payload)
        if respJson["type"] != "ok":
            print("Setting time: Remote error:", respJson["msg"])
        else:
            print("Setting time success!")
    except:
        print("No esp-smartwatch found")

# fetch all devices (=componentes, sensors) of the smartwatch every 10 seconds or every second if the last one failed
async def fetch_devices_regularly(protocol):
    global devices
    while True:
        devices = await fetch_devices(protocol)
        if devices == None:
            await asyncio.sleep(1)
        else:
            await asyncio.sleep(10)

# sets the lines specified in basestation_lines to basestation every second
async def set_basestation_lines_regularly(protocol):
    while True:
        await write_to_basestation_lcd_display(protocol, basestation_lines)
        await asyncio.sleep(1)

# sends the rows specified in rows to the base station 
async def write_to_basestation_lcd_display(protocol, rows: List[str]):
    print(f"Setting base station text to '{rows}'")
    global devices
    uri = basestation_url + "/device/lcd_display_1/command"
    payload = json.dumps({"command": "set_text", "rows": rows})
    req = aiocoap.Message(code=aiocoap.Code.POST, uri=uri, payload=payload.encode("utf-8"))
    try:
        resp : aiocoap.Message = await protocol.request(req).response
        respJson = json.loads(resp.payload)
        if respJson["type"] != "ok":
            print("Remote error: ", respJson["msg"])
        else:
            print("Setting text success!")    
    except:
        print("Failed to set text on base station")

# This contains the main application loop programmed in asynchronous manner. This allows some tasks to be executed
# in the background without blocking the main loop.
async def main():
    global devices
    global basestation_lines
    async with InfluxDBClientAsync(url="http://localhost:8086", token=token, org=org) as client:
        write_api = client.write_api()
        protocol = await aiocoap.Context.create_client_context()

        set_basestation_lines_task = asyncio.create_task(set_basestation_lines_regularly(protocol))
        while True:
            tasks = []
            if devices != None:
                print("Sending all device gets...")
            else:
                print("No device yet...")
                basestation_lines = ["Watch disconnected"]
                devices = await fetch_devices(protocol)
                if devices is None:
                    await asyncio.sleep(1)
                else:
                    # set time on smartwatch
                    asyncio.create_task(write_to_smartwatch_rtc(protocol, int(calendar.timegm(time.localtime()))))
                continue
            
            all_device_responses: List = []
            if use_block_transfers:
                # request all sensors at once
                req = aiocoap.Message(code=aiocoap.Code.GET, uri=smartwatch_url + "/all_device_values")
                try:
                    response = await protocol.request(req).response
                except:
                    print("Failed to fetch all devices")
                    continue
                
                for part in response.payload[:-1].split(b'\0'):
                    response = json.loads(part)
                    name = response["device_name"]
                    type = response["device_type"]
                    del response["device_name"]
                    del response["device_type"]
                    all_device_responses.append((name, type, response))
            else:
                # create one request for each sensor
                for device_name, device_description in devices.items():
                    req = aiocoap.Message(code=aiocoap.Code.GET, uri=smartwatch_url + "/device/" + device_name + "/value")
                    tasks.append((device_name, device_description["type"], protocol.request(req).response))

                
                for device_name, device_type, response in tasks:
                    try:
                        response = await response
                    except:
                        print(f"Failed to fetch {device_name}")
                        continue
                    response = json.loads(response.payload)
                    all_device_responses.append((device_name, device_type, response))

            # now that we have all response, process them!
            points = []
            local_basestation_lines = []
            for device_name, device_type, response in all_device_responses:
                if "type" in response and response["type"] == "error":
                    print(f"Sensor {device_name} error: {response}")
                else:
                    point = Point(device_type).tag("deviceName", device_name)
                    for measurement_name, measurement_value in response.items():
                        point = point.field(measurement_name, float(measurement_value))
                        
                        # create rows for base station
                        if measurement_name in rename_map.keys():
                            if measurement_name in integer_vars:
                                local_basestation_lines.append(f"{rename_map[measurement_name]}: {measurement_value}")
                            else:
                                local_basestation_lines.append(f"{rename_map[measurement_name]}: {measurement_value:.2f}")
                    points.append(point)
            
            basestation_lines = local_basestation_lines
            if len(points) != 0:
                # write the data back to influx in the background
                asyncio.create_task(write_api.write(bucket, org, record=points))

            # This allows pending tasks to be executed.
            await asyncio.sleep(0)

asyncio.run(main())
