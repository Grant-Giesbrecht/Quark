import os
from colorama import Fore, Style

# Reads a configuration file that consists of keys and values separated by an
# equals sign. Comments are anything that appear behind double backslashes, like
# in C.
#
# Returns the number of items added to the settings dictionary
def load_conf(filename:str, settings:dict):

	num_added = 0

	with open(filename, 'r') as f:
		for row in f:

			# Remove comments
			idx = row.find("//")
			if idx != -1:
				row = row[0:idx].strip()

			# Find delimiter
			idx = row.find("=")
			if idx != -1:
				k = row[0:idx].strip()
				v = row[idx+2:].strip()
				settings[k]=v
				num_added += 1

	return num_added

# Accepts a string in the format 'x.y.z' and returns a tuple with (x,y,z). If string
# is not in this format or x,y,z not all ints, returns None.
def versionstr_to_int(name):

	# Find delimiters, abort if can't find
	idx0 = name.find(".")
	if idx0 == -1:
		return None
	idx1 = name.find(".", idx0+1)
	if idx1 == -1:
		return None

	# Read version info
	series = name[0:idx0]
	maj, min, patch = 0,0,0
	try:
		maj = int(name[0:idx0])
		min = int(name[idx0+1:idx1])
		patch = int(name[idx1+1:])
	except:
		return None

	return (maj, min, patch)

# Given the path to the ISV_Archive directory, it retuns a dictionary with each
# key being a series, and each value being a list of tuples of major, minor, patch
#
def get_all_ISV(archive_path:str):

	all_isv = dict()

	# Get contents
	cont = os.listdir(archive_path)

	# For each item...
	for name in cont:
		#Get full path to item
		item = os.path.join(archive_path, name)
		# Check if is directory
		if os.path.isdir(item):

			# Find delimiters, abort if can't find
			idx0 = name.find("_")
			if idx0 == -1:
				continue
			idx1 = name.find("_", idx0+1)
			if idx1 == -1:
				continue
			idx2 = name.find("_", idx1+1)
			if idx2 == -1:
				continue

			# Read version info
			series = name[0:idx0]
			maj, min, patch = 0,0,0
			try:
				maj = int(name[idx0+1:idx1])
				min = int(name[idx1+1:idx2])
				patch = int(name[idx2+1:])
			except:
				continue

			# Save ISV info
			if series not in all_isv:
				all_isv[series] = []
			all_isv[series].append( (maj, min, patch) )

	# Sort resuls
	sers = all_isv.keys()
	for series in sers:
		all_isv[series] = sorted(all_isv[series], key=lambda tup: tup[2]) # Sort by patch
		all_isv[series] = sorted(all_isv[series], key=lambda tup: tup[1]) # Sort by minor
		all_isv[series] = sorted(all_isv[series], key=lambda tup: tup[0]) # Sort by major

	return all_isv

# Accepts dict of ISV versions (ie. from get_all_ISV) and prints them
def show_ISVs(all_isv):

	serieses = all_isv.keys()

	# For each series
	for series in serieses:
		print(f"Series: {Fore.BLUE}{series}{Style.RESET_ALL}")

		# FOr each version
		for isv in all_isv[series]:
			maj = isv[0]
			min = isv[1]
			patch = isv[2]
			print(f"\t{maj}.{min}.{patch}")

# Takes a dict of ISVs (as created by get_all_ISV), and runs a menu to prompt the
# user through selecting one. Returns a string for the selected versions directory
# name, or "" if no version can be selected (empty archive).
#
def get_isv_menu(all_isv):

	# Show archive contents
	print(f"{Fore.YELLOW}Instruction Sets in Archive{Style.RESET_ALL}")
	show_ISVs(all_isv)

	# Get series options
	k = all_isv.keys()
	k = list(k)

	ver = ""

	# Based on number of series available, either quit, select only series, or give option to pick.
	if len(k) == 1: # Pick only series
		k = k[0] # Get only series
		print(f"Selected only available series: {Fore.BLUE}{k}{Style.RESET_ALL}")
	elif len(k) == 0: # ABort
		print(f"{Fore.RED}ERROR: Archive is empty{Style.RESET_ALL}")
		return ""
	else: # Run menu

		while True:

			# Get user input
			usr_ser = input("{Fore.YELLOW}Select a series:{Style.RESET_ALL} ")

			# CHeck for blank
			if len(usr_ser) < 1:
				continue;

			maj, min, patch = -1,-1,-1
			if usr_ser in k:
				k = usr_ser
				break
			else:
				print(f"{Fore.RED}ERROR: Cannot find series '{usr_ser}'{Style.RESET_ALL}")
				continue

	# Get version
	while True:
		usr_ver = input(f"{Fore.YELLOW}Select version [format: x.x.x, use hyphen (-) for latest]:{Style.RESET_ALL} ")
		if usr_ver == "-": #Auto
			maj = all_isv[k][-1][0]
			min = all_isv[k][-1][1]
			patch = all_isv[k][-1][2]
		else:
			vers = versionstr_to_int(usr_ver)
			if vers is None:
				print(f"{Fore.RED}ERROR: Cannot invalid version string '{usr_ver}'{Style.RESET_ALL}")
				continue
			if vers not in all_isv[k]:
				print(f"{Fore.RED}ERROR: Version '{usr_ver}' does not exist.{Style.RESET_ALL}")
				continue
			maj = vers[0]
			min = vers[1]
			patch = vers[2]

		# Create version string
		ver = f"{k}_{maj}_{min}_{patch}"

		# Get confirmation
		print(f"Selected version {Fore.BLUE}{ver}{Style.RESET_ALL} ")
		usr_conf = input(f"{Fore.YELLOW}Use this version? (y/n):{Style.RESET_ALL} ")
		if usr_conf in ["y", "Y"]:
			break
		else:
			continue

	return ver
