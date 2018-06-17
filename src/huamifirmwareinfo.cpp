#include "huamifirmwareinfo.h"

HuamiFirmwareInfo::HuamiFirmwareInfo(const QByteArray &bytes)
{
    mBytes = bytes;
}

HuamiFirmwareInfo::Type HuamiFirmwareInfo::determineFirmwareType() {
    if (mBytes.startsWith(RES_HEADER) || mBytes.startsWith(NEWRES_HEADER)) {
        if (mBytes.length() > 700000) { // dont know how to distinguish from Cor .res
            return HuamiFirmwareInfo::Invalid;
        }
        return HuamiFirmwareInfo::Res;
    }
    if (mBytes.startsWith(GPS_HEADER) || mBytes.startsWith(GPS_HEADER2) || mBytes.startsWith(GPS_HEADER3) || mBytes.startsWith(GPS_HEADER4)) {
        return HuamiFirmwareInfo::GPS;
    }
    if (mBytes.startsWith(GPS_ALMANAC_HEADER)) {
        return HuamiFirmwareInfo::GPS_ALMANAC;
    }
    if (mBytes.startsWith(GPS_CEP_HEADER)) {
        return HuamiFirmwareInfo::GPS_CEP;
    }
    if (mBytes.startsWith(FW_HEADER)) {
        QString foundVersion = searchFirmwareVersion();
        if (!foundVersion.isEmpty()) {
            if ((foundVersion >= "0.0.8.00") && (foundVersion <= "1.0.5.00")) {
                return HuamiFirmwareInfo::Firmware;
            }
        }
        return HuamiFirmwareInfo::Invalid;
    }
    if (mBytes.startsWith(WATCHFACE_HEADER)) {
        return HuamiFirmwareInfo::Watchface;
    }
    if (mBytes.startsWith(NEWFT_HEADER)) {
        if (mBytes[10] == 0x01) {
            return HuamiFirmwareInfo::Font;
        } else if (mBytes[10] == 0x02) {
            return HuamiFirmwareInfo::Font_Latin;
        }
    }
    return HuamiFirmwareInfo::Invalid;
}

QString HuamiFirmwareInfo::searchFirmwareVersion() {
    /*
    ByteBuffer buf = ByteBuffer.wrap(fwbytes);
    buf.order(ByteOrder.BIG_ENDIAN);
    while (buf.remaining() > 3) {
        int word = buf.getInt();
        if (word == 0x5625642e) {
            word = buf.getInt();
            if (word == 0x25642e25) {
                word = buf.getInt();
                if (word == 0x642e2564) {
                    word = buf.getInt();
                    if (word == 0x00000000) {
                        byte version[] = new byte[8];
                        buf.get(version);
                        return new String(version);
                    }
                }
            }
        }
    }
    */
    return QString();
}
