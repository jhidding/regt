#include <gtest/gtest.h>
#include "support/ply/ply.hh"
#include "generate-wave.hh"

TEST(Ply, WritingASCII)
{
	PLY::PLY ply(PLY::ASCII);
	generate_wave(ply);
    ply.save("ply_test_ascii.ply");
}

TEST(Ply, Binary)
{
    PLY::PLY ply(PLY::BINARY);
    generate_wave(ply);
    ply.save("ply_test_binary.ply");

    PLY::PLY ply2("ply_test_binary.ply");

    ASSERT_TRUE(ply2.check());
    ASSERT_EQ(ply2["vertex"].size(), ply["vertex"].size());
    ASSERT_EQ(ply2["face"].size(), ply["face"].size());
}

TEST(Ply, FaultyBinary)
{
    PLY::PLY ply(PLY::BINARY);
    generate_wave(ply);

    std::ofstream f("ply_test_faulty.ply");
    f << ply.header();
    f.write(ply["vertex"].data(), ply["vertex"].byte_size());
    // only write half of the face data
    f.write(ply["face"].data(), ply["face"].byte_size() / 2);
    f.close();

    ASSERT_THROW(
        PLY::PLY ply2("ply_test_faulty.ply"),
        PLY::Exception);
}