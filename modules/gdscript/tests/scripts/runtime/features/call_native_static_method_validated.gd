func test():
	# Validated native static call with return value.
	print(FileAccess.file_exists("some_file"))

	# Validated native static call without return value.
	# Keep this independent of orphan Nodes registered by custom modules.
	PortableCompressedTexture2D.set_keep_all_compressed_buffers(PortableCompressedTexture2D.is_keeping_all_compressed_buffers())
