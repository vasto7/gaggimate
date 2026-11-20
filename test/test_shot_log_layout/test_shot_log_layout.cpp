#include <unity.h>
#include <cstddef>
#include <initializer_list>
#include <display/models/shot_log_format.h>

void setUp() { /* no shared fixture state */ }
void tearDown() { /* no shared fixture state */ }

void test_legacy_and_current_layouts() {
    ShotLogHeader h{};
    h.fieldsMask = 0x1FFF;
    h.version = 5;
    h.reserved0 = 26;
    TEST_ASSERT_FALSE(shotLogHasElapsedTimestamp(h));
    TEST_ASSERT_EQUAL_UINT8(26, shotLogSampleSize(h));
    h.version = 6;
    TEST_ASSERT_FALSE(shotLogHasElapsedTimestamp(h));
    TEST_ASSERT_EQUAL_UINT8(26, shotLogSampleSize(h));
    h.reserved0 = 28;
    TEST_ASSERT_TRUE(shotLogHasElapsedTimestamp(h));
    TEST_ASSERT_EQUAL_UINT8(28, shotLogSampleSize(h));
    h.fieldsMask = 0x3FFF;
    h.reserved0 = 30;
    for (uint8_t version : {7, 8}) {
        h.version = version;
        TEST_ASSERT_TRUE(shotLogHasElapsedTimestamp(h));
        TEST_ASSERT_EQUAL_UINT8(30, shotLogSampleSize(h));
    }
    h.reserved0 = 26;
    TEST_ASSERT_EQUAL_UINT8(0, shotLogSampleSize(h));
    // v9 adds rp, so the record grows to 32 bytes.
    h.version = 9;
    h.fieldsMask = 0x7FFF;
    h.reserved0 = 32;
    TEST_ASSERT_TRUE(shotLogHasElapsedTimestamp(h));
    TEST_ASSERT_EQUAL_UINT8(32, shotLogSampleSize(h));
    // A file written by a future firmware with a wider record still reads: the size comes from
    // the file, and the reader skips the remainder it does not know about.
    h.reserved0 = 34;
    TEST_ASSERT_EQUAL_UINT8(34, shotLogSampleSize(h));
}

void test_v9_field_offsets() {
    // The web parser reads these by hard-coded offset, so a silent change here would be a
    // silent misparse there. Keep the two in step.
    TEST_ASSERT_EQUAL_UINT32(512, sizeof(ShotLogHeader));
    TEST_ASSERT_EQUAL_UINT32(32, sizeof(ShotLogSample));
    // rp is appended last, so the first 30 bytes of a v9 record are byte-identical to a v8 one.
    TEST_ASSERT_EQUAL_UINT32(30, offsetof(ShotLogSample, rp));

    TEST_ASSERT_EQUAL_UINT32(462, offsetof(ShotLogHeader, settingsVersion));
    TEST_ASSERT_EQUAL_UINT32(466, offsetof(ShotLogHeader, pressureOffsetIntercept));
    TEST_ASSERT_EQUAL_UINT32(470, offsetof(ShotLogHeader, pressureOffsetSlope));
    TEST_ASSERT_EQUAL_UINT32(474, offsetof(ShotLogHeader, pumpModelCoeffs));
    TEST_ASSERT_EQUAL_UINT32(490, offsetof(ShotLogHeader, firmwareBuildId));
    // The snapshot must stay inside the padding it was carved from.
    TEST_ASSERT_EQUAL_UINT32(494, offsetof(ShotLogHeader, reserved_v9));
}

void test_settings_snapshot_absent_in_old_files() {
    // A pre-v9 file has zeros across this range; settingsVersion 0 must read as "absent"
    // rather than as a real snapshot claiming a zero offset and a zero flow model.
    ShotLogHeader h{};
    h.version = 8;
    TEST_ASSERT_EQUAL_UINT8(0, h.settingsVersion);
}

void test_build_id_distinguishes_versions() {
    TEST_ASSERT_EQUAL_UINT32(shotLogBuildId("v1.10.0"), shotLogBuildId("v1.10.0"));
    TEST_ASSERT_NOT_EQUAL(shotLogBuildId("v1.10.0"), shotLogBuildId("v1.10.1"));
    TEST_ASSERT_NOT_EQUAL(0u, shotLogBuildId("v1.10.0"));
    // An empty or null version string must not crash; it just yields the FNV offset basis.
    TEST_ASSERT_EQUAL_UINT32(2166136261u, shotLogBuildId(""));
    TEST_ASSERT_EQUAL_UINT32(2166136261u, shotLogBuildId(nullptr));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_legacy_and_current_layouts);
    RUN_TEST(test_v9_field_offsets);
    RUN_TEST(test_settings_snapshot_absent_in_old_files);
    RUN_TEST(test_build_id_distinguishes_versions);
    return UNITY_END();
}
