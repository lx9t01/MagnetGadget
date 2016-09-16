#############################################
##     Record Serial Data from USB port    ##
##          by Haixiang Xu                 ##
##          Note: requires serial lib      ##
#############################################


import serial
import matplotlib.pyplot as plt


delay_ms = 20 # sample rate 50 Hz
update_ms = 1000 # update period = 1 s
plot_ms = 5000 # refresh period = 5 s
connected = False
locations = ['/dev/tty.usbmodem1421','/dev/tty.usbmodem1411','/dev/ttyUSB0','/dev/ttyUSB1','/dev/ttyUSB2','/dev/ttyUSB3','dev/tty.usbserial','/dev/cu.usbmodem1421']

for device in locations:
    try:
        print "Trying...",device
        ser = serial.Serial(device, 9600)
        break
    except:
        print "Failed to connect on",device

while not connected:
    serin = ser.read()
    connected = True;

outfile = open("result.txt", 'w');

############### record plot ###############
time = []
data = [[],[],[],[]]
s = ""

plt.ioff()
fig = plt.figure(num=1)
lnx, = plt.plot(time, data[0], 'r-', label='x')
lny, = plt.plot(time, data[1], 'g-', label='y')
lnz, = plt.plot(time, data[2], 'b-', label='z')
lnt, = plt.plot(time, data[3], 'k-', label='intensity')
plt.legend([lnx, lny, lnz, lnt], ['X', 'Y', 'Z', 'Total'])
fig.show()
plt.xlabel('time (ms)')
plt.ylabel('magnet field intendity (micro Tesla)')
plt.grid()
max_range = 300.0
idx = 1

while 1:
    try:
        if ser.inWaiting():
            x=ser.read()
            outfile.write(x)
            s = s + x
            if x=="\n":
                print s
                try:
                    [t, nx, ny, nz, nt] = s.split(' ')
                    # print(t+" "+nx+" "+ny+" "+nz+" "+nt)
                    time.append(int(t))
                    data[0].append(float(nx))
                    data[1].append(float(ny))
                    data[2].append(float(nz))
                    data[3].append(float(nt))
                    max_range = max(max_range, abs(float(nt)))
                except ValueError:
                    pass
                s = ""
                idx += 1
                outfile.flush()

            if idx % (update_ms / delay_ms) == 0: # 0.5s update
                lnx.set_xdata(time)
                lnx.set_ydata(data[0])
                lny.set_xdata(time)
                lny.set_ydata(data[1])
                lnz.set_xdata(time)
                lnz.set_ydata(data[2])
                lnt.set_xdata(time)
                lnt.set_ydata(data[3])
                plt.axis([time[0], time[0] + plot_ms, -max_range, max_range])
                plt.draw()
                plt.pause(0.005)

            if idx % (plot_ms / delay_ms) == 0: # 5s refresh
                fig = plt.clf()
                time = []
                data = [[],[],[],[]]
                lnx, = plt.plot(time, data[0], 'r-', label='x')
                lny, = plt.plot(time, data[1], 'g-', label='y')
                lnz, = plt.plot(time, data[2], 'b-', label='z')
                lnt, = plt.plot(time, data[3], 'k-', label='intensity')
                plt.legend([lnx, lny, lnz, lnt], ['X', 'Y', 'Z', 'Total'])
                plt.xlabel('time (ms)')
                plt.ylabel('magnet field intendity (micro Tesla)')
                plt.grid()
                idx = 1
                max_range = 300.0
    except KeyboardInterrupt:
        print "unplugged" 
        break;

# close the serial connection and text file
outfile.close()
ser.close()

