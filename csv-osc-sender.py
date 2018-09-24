import argparse
import csv
from typing import List, Any

from pythonosc import dispatcher
from pythonosc import osc_server
from pythonosc import osc_message_builder
from pythonosc import udp_client

step_count = 0


def initCSV(index):
    fileName = ""
    if index == 0:
        fileName = "" #filename
    elif index == 1:
        fileName = "" #filename
    elif index == 2:
        fileName = "" #filename
    elif index == 3:
        fileName = "" #filename ./data-logs/filename
    else:
        fileName = "noname"

    with open(fileName) as csvfile:
        readCSV = csv.reader(csvfile, delimiter=';')
        row_count = sum(1 for row in csvfile) - 1 #subtract 1 for header
        csvfile.seek(0)

        readCSV.__next__()
        latitudes = []
        longitudes = []
        n0list = []
        n1list = []
        n2list = []
        unixes = []
        ymds = []
        ywds = []
        hmss = []
        zmds = []
        ccsCO2s = []
        ccsTVOCs = []
        silTemps = []
        silHumids = []
        mplPresses = []
        mplTemps = []
        soilMoists = []

        dataArray = []

        for row in readCSV:
            latitude = row[0]
            longitude = row[1]
            n0dat = row[2]
            n1dat = row[3]
            n2dat = row[4]
            unix = row[5]
            ymd = row[6]
            ywd = row[7]
            hms = row[8]
            zmd = row[9]
            ccsCO2 = row[10]
            ccsTVOC = row[11]
            silTemp = row[12]
            silHumid = row[13]
            mplPress = row[14]
            mplTemp = row[15]
            soilMoist = row[16]

            latitudes.append(latitude)
            longitudes.append(longitude)
            n0list.append(n0dat)
            n1list.append(n1dat)
            n2list.append(n2dat)
            unixes.append(unix)
            ymds.append(ymd)
            ywds.append(ywd)
            hmss.append(hms)
            zmds.append(zmd)
            ccsCO2s.append(ccsCO2)
            ccsTVOCs.append(ccsTVOC)
            silTemps.append(silTemp)
            silHumids.append(silHumid)
            mplPresses.append(mplPress)
            mplTemps.append(mplTemp)
            soilMoists.append(soilMoist)

            dataArray.append(latitudes)
            dataArray.append(longitudes)
            dataArray.append(n0list)
            dataArray.append(n1list)
            dataArray.append(n2list)
            dataArray.append(unixes)
            dataArray.append(ymds)
            dataArray.append(ywds)
            dataArray.append(hmss)
            dataArray.append(zmds)
            dataArray.append(ccsCO2s)
            dataArray.append(ccsTVOCs)
            dataArray.append(silTemps)
            dataArray.append(silHumids)
            dataArray.append(mplPresses)
            dataArray.append(mplTemps)
            dataArray.append(soilMoists)

    return fileName, dataArray, row_count


def sendMessage(address, label, data):
    global step_count
    global dataArrayed
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip", default="127.0.0.1",
                        help="The ip of the OSC server")
    parser.add_argument("--port", type=int, default=1068,
                        help="The port the OSC server is listening on")
    server_args = parser.parse_args()
    client = udp_client.SimpleUDPClient(server_args.ip, server_args.port)

    try:
        if str(address) == "/index/step" and str(data) == "trigger":
            print("Step: " + str(step_count))
            client.send_message("/geoCoord/latitude", float(dataArrayed[0][step_count]))
            client.send_message("/geoCoord/longitude", str(dataArrayed[1][step_count]))
            client.send_message("/geoCoord/nVector/n0", float(dataArrayed[2][step_count]))
            client.send_message("/geoCoord/nVector/n1", float(dataArrayed[3][step_count]))
            client.send_message("/geoCoord/nVector/n2", float(dataArrayed[4][step_count]))
            client.send_message("/time/unix", float(dataArrayed[5][step_count]))
            client.send_message("/time/yyyy-mm-dd", str(dataArrayed[6][step_count]))
            client.send_message("/time/yyyy-ww-ddd", str(dataArrayed[7][step_count]))
            client.send_message("/time/hh:mm:ss", str(dataArrayed[8][step_count]))
            client.send_message("/time/zone-month-day", str(dataArrayed[9][step_count]))
            client.send_message("/data/ccs811CO2", float(dataArrayed[10][step_count]))
            client.send_message("/data/ccs811TVOC", float(dataArrayed[11][step_count]))
            client.send_message("/data/silTemperature", float(dataArrayed[12][step_count]))
            client.send_message("/data/silHumidity", float(dataArrayed[13][step_count]))
            client.send_message("/data/mplPressure", float(dataArrayed[14][step_count]))
            client.send_message("/data/mplTemperature", float(dataArrayed[15][step_count]))
            client.send_message("/data/soilMoisture", float(dataArrayed[16][step_count]))
            client.send_message("/list/step-count", float(step_count))
            step_count += 1
        elif str(address) == "/index/set" and type(data) is int:
            print("Set: " + str(step_count))
            client.send_message("/geoCoord/latitude", float(dataArrayed[0][data]))
            client.send_message("/geoCoord/longitude", str(dataArrayed[1][data]))
            client.send_message("/geoCoord/nVector/n0", float(dataArrayed[2][data]))
            client.send_message("/geoCoord/nVector/n1", float(dataArrayed[3][data]))
            client.send_message("/geoCoord/nVector/n2", float(dataArrayed[4][data]))
            client.send_message("/time/unix", float(dataArrayed[5][data]))
            client.send_message("/time/yyyy-mm-dd", str(dataArrayed[6][data]))
            client.send_message("/time/yyyy-ww-ddd", str(dataArrayed[7][data]))
            client.send_message("/time/hh:mm:ss", str(dataArrayed[8][data]))
            client.send_message("/time/zone-month-day", str(dataArrayed[9][data]))
            client.send_message("/data/ccs811CO2", float(dataArrayed[10][data]))
            client.send_message("/data/ccs811TVOC", float(dataArrayed[11][data]))
            client.send_message("/data/silTemperature", float(dataArrayed[12][data]))
            client.send_message("/data/silHumidity", float(dataArrayed[13][data]))
            client.send_message("/data/mplPressure", float(dataArrayed[14][data]))
            client.send_message("/data/mplTemperature", float(dataArrayed[15][data]))
            client.send_message("/data/soilMoisture", float(dataArrayed[16][data]))
            client.send_message("/list/step-count", float(step_count))
            step_count = data + 1
        elif str(address) == "/index/insert" and type(data) is int:
            print("Insert: Index " + str(data))
            client.send_message("/geoCoord/latitude", float(dataArrayed[0][data]))
            client.send_message("/geoCoord/longitude", str(dataArrayed[1][data]))
            client.send_message("/geoCoord/nVector/n0", float(dataArrayed[2][data]))
            client.send_message("/geoCoord/nVector/n1", float(dataArrayed[3][data]))
            client.send_message("/geoCoord/nVector/n2", float(dataArrayed[4][data]))
            client.send_message("/time/unix", float(dataArrayed[5][data]))
            client.send_message("/time/yyyy-mm-dd", str(dataArrayed[6][data]))
            client.send_message("/time/yyyy-ww-ddd", str(dataArrayed[7][data]))
            client.send_message("/time/hh:mm:ss", str(dataArrayed[8][data]))
            client.send_message("/time/zone-month-day", str(dataArrayed[9][data]))
            client.send_message("/data/ccs811CO2", float(dataArrayed[10][data]))
            client.send_message("/data/ccs811TVOC", float(dataArrayed[11][data]))
            client.send_message("/data/silTemperature", float(dataArrayed[12][data]))
            client.send_message("/data/silHumidity", float(dataArrayed[13][data]))
            client.send_message("/data/mplPressure", float(dataArrayed[14][data]))
            client.send_message("/data/mplTemperature", float(dataArrayed[15][data]))
            client.send_message("/data/soilMoisture", float(dataArrayed[16][data]))
            client.send_message("/list/step-count", float(step_count))
        elif str(address) == '/list/select':
            print("List select: " + str(data))
            verify = initCSV(data)
            name = verify[0]
            dataArrayed = verify[1]
            length = verify[2]
            client.send_message("/list/select", str(name))
            client.send_message("/list/length", float(length))
            print("File name: " + str(name) + " ||| File length: " + str(length))
    except IndexError:
        print("List end.")
        client.send_message("/list/end", "bang")
    except FileNotFoundError:
        print("List select: File not found. Check index listing.")
        client.send_message("/list/select", "File not found. Check index listing.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip",
                        default="127.0.0.1", help="The ip to listen on")
    parser.add_argument("--port",
                        type=int, default=5008, help="The port to listen on")
    args = parser.parse_args()

    dispatcher = dispatcher.Dispatcher()
    dispatcher.map('/index/step', sendMessage, 'Step')
    dispatcher.map('/index/set', sendMessage, 'Set')
    dispatcher.map('/index/insert', sendMessage, 'Insert')
    dispatcher.map('/list/select', sendMessage, 'List select')

    server = osc_server.ThreadingOSCUDPServer(
        (args.ip, args.port), dispatcher)
    print(f'Serving on {server.server_address}')
    server.serve_forever()

