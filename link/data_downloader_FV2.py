# from IPython import get_ipython
#
# try:
#     __IPYTHON__
# except NameError:
# 	print("Not in IPython")
# else:
# 	get_ipython().run_line_magic("reset", "-f")
# 	print("In IPython")

import pyvisa as pv
import time

# filename = "../Blinkenmatrix/opfiles/memorydelta_downlink.bcm"
# readlength = 1001;
filename = "./Example Data/blinken_downlink.mcb"
readlength = 4359;
readlength = 30;

lines_per_batch = 512

t_upload = []
t_write = []

recd_addrs = []
recd_data = []

read_finished = False;

#Open connection to Arduino
rm = pv.ResourceManager()
try:
    addr = rm.list_resources()[1]
    dev = rm.open_resource(addr, baud_rate = 115200)
    # dev.baud_rate = 115200
except Exception as e:
    print("Failed to open device")
    print(str(e))
    exit()

def main():


    # dev.timeout = 25e3 #Timeout in ms

    #Print messages
    print(f"Aquired device resource.\n\tAddress: \"{addr}\"")
    print("Waiting to connect", end="", flush=True)

    #Send messages to device and wait until it responds...
    connected = False
    for i in range(10):

        dev.timeout = 1e3
        dev.write("R")
        try:
            dev.read()
        except pv.errors.VisaIOError:
            print('.', end="", flush=True)
        else:
            connected = True
            print("Connection verified - continuing\n")
            break;
    if not connected:
        print("\nFailed to verify connection. Exiting.")
        exit()

    #Read messages from the device just to make sure the buffer is clear
    print("Clearing buffer", end="", flush=True)
    num_reads = 0;
    while (True):
        try:
            dev.read()
        except pv.errors.VisaIOError:
            break;
        else:
            num_reads += 1;
    if num_reads == 0:
        print(" \t\tNothing to clear. Continuing")
    else:
        print(f" \t\tCleared {num_reads} messages")

    #Set timeout to be longer
    dev.timeout = 10e3 #ms

    print(f"\nDownloading data to file: {filename}")

    # Send packet length - request read operation
    print(f"\nRequesting read of length: {readlength}", end='', flush=True)
    dev.write(f"L={readlength}*")
    recd = dev.read().strip("\n").strip('\r')
    #
    #This double checks that there aren't some stray special chars in string
    orig_len = len(recd)
    recd = recd.replace('\n', '')
    recd = recd.replace('\r', '')
    had_bad = False
    if (orig_len != len(recd)):
        had_bad = True
    #
    if recd[0] == "G":
        if (had_bad):
            print("!", end='')
        print("\tAcknowledged by Arduino")
        print(f"\t\tBeginning read of length: {recd[1:]}")
    else:
        if (had_bad):
            print("!", end='')
        print(f"? \t'{recd}'")
        exit();

    print("Beginning read")

    while (True):

        listen_to_chipread()

        if not get_downlink():
            break;



    print("\n\nRead complete")
    print(f"\nSaving file {filename}")
    save_to_file(filename)

def listen_to_chipread():
    print("Arduino is reading chip.")
    print('\n|           | (* = 50 Bytes written)\r|', end='', flush=True);
    while(True):

        recd = dev.read().strip('\n').strip('\r')
        if recd == "Ready":
            print("\nWrite complete. Sending next batch of data\n")
            return;
        elif recd == "U":
            print("*", end='', flush=True)
        else:
            print(f"?({recd})", end='', flush=True)
    print("Chip read phase complete")

def get_downlink():

    quit_after = False;

    print("Downlink in process")
    while (True):

        recd = dev.read().strip("\n").strip('\r')

        if recd == "E":
            break;

        if recd == "D":
            quit_after = True;
            break;

        if process_data(recd):
            dev.write("G")
        else:
            dev.write("B")
    print("Downlink complete")

    return not quit_after


def process_data(x:str):

    print(x)

    try:
        addr, data = x.split(":")
        addr = int(addr)
        data = int(data)
    except Exception as e:
        print(f"Packet failed! ({x})")
        # print(str(e))
        return False;

    recd_addrs.append(addr)
    recd_data.append(data)


def save_to_file(save_filename:str):

    if (len(recd_data) != len(recd_addrs)):
        print("Wrong address length")
        return

    with open(save_filename, 'w') as of:
        for idx, a in enumerate(recd_addrs):
            d = recd_data[idx]
            of.write(f'{a}:{d}\n')

if __name__ == '__main__':
    main()
