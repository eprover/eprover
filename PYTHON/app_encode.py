def main():
	import sys, os

	app_encode_bin = sys.argv[1]
	in_folder      = sys.argv[2]
	out_folder     = sys.argv[3]

	in_files = [os.path.join(in_folder,f) for f in os.listdir(in_folder) \
				if os.path.isfile(os.path.join(in_folder, f))]

	for in_file in in_files:
		os.system("{0} {1} 2> {2}".format(app_encode_bin, in_file, 
										  os.path.join(out_folder, os.path.basename(in_file))))

if __name__ == '__main__':
	main()