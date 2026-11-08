#include "tests/test_macros.h"

TEST_FORCE_LINK(test_ph_zip)

#if defined(MODULE_HBNATIVE_ENABLED) && defined(MINIZIP_ENABLED)

#include "modules/hbnative/ph_zip.h"
#include "modules/hbnative/ph_zip_packer.h"
#include "tests/test_utils.h"

namespace TestPHZip {

TEST_CASE("[PHZip] writes and reads ZIP64 entry headers") {
	const String path = TestUtils::get_temp_path("ph_zip_zip64.zip");
	Ref<PHZIPPacker> packer;
	packer.instantiate();
	CHECK(packer->open(path, PHZIPPacker::APPEND_CREATE) == OK);
	CHECK(packer->start_file("small.txt") == OK);
	Vector<uint8_t> data;
	data.push_back('z');
	data.push_back('i');
	data.push_back('p');
	CHECK(packer->write_file(data) == OK);
	CHECK(packer->close_file() == OK);
	CHECK(packer->close() == OK);

	Ref<FileAccess> raw = FileAccess::open(path, FileAccess::READ);
	CHECK(raw.is_valid());
	PackedByteArray local_header = raw->get_buffer(8);
	REQUIRE(local_header.size() == 8);
	CHECK(local_header[0] == 'P');
	CHECK(local_header[1] == 'K');
	CHECK(local_header[2] == 3);
	CHECK(local_header[3] == 4);
	// zipOpenNewFileInZip4_64(..., zip64 = 1) reserves ZIP64 local-header data.
	CHECK(local_header[4] == 45);
	CHECK(local_header[5] == 0);

	Ref<PHZipArchive> archive;
	archive.instantiate();
	CHECK(archive->try_open_pack(path, false, 0));
	CHECK(archive->file_exists("small.txt"));
	Ref<FileAccess> file = archive->get_file("small.txt");
	REQUIRE(file.is_valid());
	CHECK(file->get_length() == 3);
	CHECK(file->get_as_utf8_string() == "zip");
}

TEST_CASE("[PHZip] preserves long UTF-8 entry names") {
	const String name = String("directory/") + String::utf8("\xC3\xA9").repeat(150) + ".txt";
	const String path = TestUtils::get_temp_path("ph_zip_long_name.zip");
	Ref<PHZIPPacker> packer;
	packer.instantiate();
	REQUIRE(packer->open(path, PHZIPPacker::APPEND_CREATE) == OK);
	REQUIRE(packer->start_file(name) == OK);
	Vector<uint8_t> data;
	data.push_back(42);
	REQUIRE(packer->write_file(data) == OK);
	REQUIRE(packer->close_file() == OK);
	REQUIRE(packer->close() == OK);

	Ref<PHZipArchive> archive;
	archive.instantiate();
	REQUIRE(archive->try_open_pack(path, false, 0));
	CHECK(archive->file_exists(name));
}

} // namespace TestPHZip

#endif
