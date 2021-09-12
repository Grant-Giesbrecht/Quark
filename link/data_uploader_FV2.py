import pyvisa as pv
import time

filename = "./Example Data/add.mcb"
lines_per_batch = 512

t_upload = []
t_write = []

port = "FRONT_LEFT"; # Options: BACK_LEFT, FRONT_LEFT, BACK_RIGHT, FRONT_RIGHT, FIRST

chip_erase = True

#Open connection to Arduino
rm = pv.ResourceManager()
try:
	if port == "FIRST":
		addr_list = rm.list_resources()[1]
	else:
		addr_list = rm.list_resources()
		addr = "INVALID"
		for ad in addr_list:
			if port == "BACK_LEFT":
				if '14401' in ad:
					addr = ad
			elif port == "FRONT_LEFT":
				if '14301' in ad:
					addr = ad
			elif port == "FRONT_RIGHT":
				if '14201' in ad:
					addr = ad
			elif port == "BACK_RIGHT":
				if '14101' in ad:
					addr = ad
	dev = rm.open_resource(addr, baud_rate = 115200)
	# dev.baud_rate = 115200
except Exception as e:
	print("Failed to open device")
	print(str(e))
	exit()
# dev.timeout = 25e3 #Timeout in ms

#Print messages
print(f"Aquired device resource.\n\tAddress: \"{addr}\"")
print("Waiting to connect", end="", flush=True)

#Send messages to device and wait until it responds...
connected = False
for i in range(10):

	dev.timeout = 1e3
	if chip_erase:
		dev.write("C")
	else:
		dev.write("T")
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
dev.timeout = 3e3 #ms

print(f"\nUploading file: {filename}")

#Open file
line_count = 0
batch_no = -1
wrote_set = False
with open(filename, 'r') as f:

	start = time.time()

	#For each line
	for line in f:

		batch_no += 1

		wrote_set = False

		line = line.strip('\n')

		#Send line to Arduino
		print(f"Sending '{line}' \t\t", end="", flush=True)
		dev.write(f"{line}*")
		recd = dev.read().strip("\n").strip('\r')

		#This double checks that there aren't some stray special chars in string
		orig_len = len(recd)
		recd = recd.replace('\n', '')
		recd = recd.replace('\r', '')
		had_bad = False
		if (orig_len != len(recd)):
			had_bad = True

		if recd == "G":
			if (had_bad):
				print("!", end='')
			print("Good")
		else:
			if (had_bad):
				print("!", end='')
			print(f"? \t'{recd}'")

		#Wait for Arduino to sync if neccesary
		line_count += 1;



		if line_count >= lines_per_batch:

			wrote_set = True
			t_upload.append(time.time() - start)
			start = time.time()

			t_last = t_upload[-1]
			print(f"Batch {batch_no} sent. ({line_count} lines in {round(t_last, 3)} sec)")
			print("Waiting for data write to be complete")

			print('\n|           | (* = 50 Bytes written)\r|', end='', flush=True);
			while(True):

				recd = dev.read().strip('\n').strip('\r')
				if recd == "Ready":
					print("\nWrite complete. Sending next batch of data\n")
					break;
				elif recd == "U":
					print("*", end='', flush=True)
				else:
					print("?", end='', flush=True)


			line_count = 0;
			t_write.append(time.time()-start)
			start = time.time()

# If an odd number of upload cycles occured, force a write and monitor it here
if (not wrote_set):
	dev.write("END*")

	t_upload.append(time.time() - start)
	start = time.time()

	t_last = t_upload[-1]
	print(f"Batch {batch_no} sent. ({line_count} lines in {round(t_last, 3)} sec)")
	print("Waiting for data write to be complete")

	print('\n|           | (* = 50 Bytes written)\r|', end='', flush=True);
	while(True):

		recd = dev.read().strip('\n').strip('\r')
		if recd == "Ready":
			print("\nWrite complete. Sending next batch of data\n")
			break;
		elif recd == "U":
			print("*", end='', flush=True)
		elif recd == "U":
			print("X", end='', flush=True)
		else:
			print(f"?>{recd}<", end='', flush=True)

	t_write.append(time.time()-start)

#Print summary data

print("Upload to Arduino times:")
for num, t in enumerate(t_upload):
	print(f"\t{num}: {round(t, 3)} sec")

print("Write to chip times:")
for num, t in enumerate(t_write):
	print(f"\t{num}: {round(t, 3)} sec")
