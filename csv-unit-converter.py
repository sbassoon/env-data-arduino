import time
import math
import csv

fileName = "" #filename

geoCoords = [57.699341,11.965619]

initDate = "2018.05.08"
initTime = "21.42.45"

combTime = initDate + " " + initTime
stripTime = time.strptime(combTime, "%Y.%m.%d %H.%M.%S")
unixTime = time.mktime(stripTime)


def findTime(rowTime, unixTime):
    stepTime = unixTime + rowTime
    restripTime = time.localtime(stepTime)

    finalTime = [float,str,str,str,str]
    finalTime[0] = stepTime
    finalTime[1] = time.strftime("%Y-%m-%d",restripTime)
    finalTime[2] = time.strftime("%Y-%W-%j",restripTime)
    finalTime[3] = time.strftime("%H:%M:%S",restripTime)
    finalTime[4] = time.strftime("%z/%B/%A",restripTime)
    return finalTime


def makeNVector(coordinates):
    nVector = [math.cos(coordinates[0])*math.cos(coordinates[1]),
               math.cos(coordinates[0])*math.sin(coordinates[1]),
               math.sin(coordinates[0])]
    return nVector


if __name__ == "__main__":
    with open(fileName) as csvfile:
        vector = makeNVector(geoCoords)

        readCSV = csv.reader(csvfile, delimiter=';')
        readCSV.__next__()
        timings = []
        data = []
        for row in readCSV:
            timing = int(row[0]) / 1000
            dat = row[1:8]

            timings.append(timing)
            data.append(dat)
        csvfile.close()

    count = 0
    with open(fileName, 'w') as file:
        file.seek(2)
        file.writelines("lat;lon;n_e_0;n_e_1;n_e_2;unixTime;yyyy-mm-dd;yyyy-ww-ddd;hh:mm:ss;zone/month/day;ccs811C02;ccs811TVOC;si7021Temperature;si7021Humidity;mpl3115A2Pressure;mpl3115A2Temperature;soilMoisture\n")
        for i in timings:
            fullTime = findTime(i, unixTime)
            prepString = (str(geoCoords[0]) + ";" + str(geoCoords[1]) + ";" +
                          str(vector[0]) + ";" + str(vector[1]) + ";" + str(vector[2]) + ";" +
                          str(fullTime[0]) + ";" + fullTime[1] + ";" + fullTime[2] + ";" + fullTime[3] + ";" + fullTime[4] + ";" +
                          str(data[count][0]) + ";" + str(data[count][1]) + ";" +
                          str(data[count][2]) + ";" + str(data[count][3]) + ";" +
                          str(data[count][4]) + ";" + str(data[count][5]) + ";" +
                          str(data[count][6]) + "\n")

            file.writelines(prepString)
            count += 1
        file.close()

    fi = open(fileName, 'rb')
    data = fi.read()
    fi.close()

    fo = open('new-' + fileName, 'wb')
    fo.write(data.replace(b'\x00', b''))
    fo.close()





