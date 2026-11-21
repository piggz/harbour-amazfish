#include "typeconversion.h"

#include <QtTest>

class TestTypeConversion : public QObject {
    Q_OBJECT

private slots:
    void fromIntLittleEndian();
    void toUnsignedFromBytes();
    void toUnsignedMovesIndex();
    void toSignedNegative();
    void roundTrip_data();
    void roundTrip();
};

void TestTypeConversion::fromIntLittleEndian()
{
    QCOMPARE(TypeConversion::fromInt8(0x1ff), QByteArray::fromHex("ff"));
    QCOMPARE(TypeConversion::fromInt16(0x1234), QByteArray::fromHex("3412"));
    QCOMPARE(TypeConversion::BEfromInt16(0x1234), QByteArray::fromHex("1234"));
    QCOMPARE(TypeConversion::fromInt24(0x123456), QByteArray::fromHex("563412"));
    QCOMPARE(TypeConversion::fromInt32(-1), QByteArray::fromHex("ffffffff"));
    QCOMPARE(TypeConversion::fromInt64(0x0102030405060708LL), QByteArray::fromHex("0807060504030201"));
}

void TestTypeConversion::toUnsignedFromBytes()
{
    QCOMPARE(TypeConversion::toUint16(char(0xff), char(0xff)), uint16_t(0xffff));
    QCOMPARE(TypeConversion::toUint16(char(0x34), char(0x12)), uint16_t(0x1234));
    QCOMPARE(TypeConversion::toUint32(char(0xff), char(0xff), char(0xff), char(0xff)), uint32_t(0xffffffff));
    QCOMPARE(TypeConversion::toUint32(char(0x78), char(0x56), char(0x34), char(0x12)), uint32_t(0x12345678));
}

void TestTypeConversion::toUnsignedMovesIndex()
{
    const QByteArray data = QByteArray::fromHex("0100020000000300");
    int idx = 0;
    QCOMPARE(TypeConversion::toUint16(data, idx), uint16_t(1));
    QCOMPARE(idx, 2);
    QCOMPARE(TypeConversion::toUint32(data, idx), uint32_t(2));
    QCOMPARE(idx, 6);
    QCOMPARE(TypeConversion::toInt16(data, idx), int16_t(3));
    QCOMPARE(idx, 8);
}

void TestTypeConversion::toSignedNegative()
{
    int idx = 0;
    QCOMPARE(TypeConversion::toInt16(QByteArray::fromHex("ffff"), idx), int16_t(-1));
    idx = 0;
    QCOMPARE(TypeConversion::toInt16(QByteArray::fromHex("0080"), idx), int16_t(-32768));
    QCOMPARE(TypeConversion::toInt32(char(0xff), char(0xff), char(0xff), char(0xff)), int32_t(-1));
    QCOMPARE(TypeConversion::toInt32(char(0x00), char(0x00), char(0x00), char(0x80)), int32_t(INT32_MIN));
}

void TestTypeConversion::roundTrip_data()
{
    QTest::addColumn<int>("value");
    QTest::newRow("zero") << 0;
    QTest::newRow("one") << 1;
    QTest::newRow("max") << INT32_MAX;
    QTest::newRow("min") << INT32_MIN;
    QTest::newRow("minus one") << -1;
}

void TestTypeConversion::roundTrip()
{
    QFETCH(int, value);
    const QByteArray bytes = TypeConversion::fromInt32(value);
    int idx = 0;
    QCOMPARE(TypeConversion::toInt32(bytes, idx), int32_t(value));
    QCOMPARE(idx, 4);
}

QTEST_GUILESS_MAIN(TestTypeConversion)
#include "tst_typeconversion.moc"
