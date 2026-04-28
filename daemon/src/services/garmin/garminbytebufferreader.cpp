#include "garminbytebufferreader.h"

GarminByteBufferReader::GarminByteBufferReader(QByteArray data) {
        this.byteBuffer = ByteBuffer.wrap(data);
    }

    public int remaining() {
        return byteBuffer.remaining();
    }

    public ByteBuffer asReadOnlyBuffer() {
        return byteBuffer.asReadOnlyBuffer();
    }

    public void setByteOrder(QSysInfo::Endian   byteOrder) {
        this.byteBuffer.order(byteOrder);
    }

    public int readByte() {
        return Byte.toUnsignedInt(byteBuffer.get());
    }

    public int getPosition() {
        return byteBuffer.position();
    }

    public void setPosition(final int position) {
        byteBuffer.position(position);
    }

    public int getLimit() {
        return byteBuffer.limit();
    }

    public int readShort() {
        return Short.toUnsignedInt(byteBuffer.getShort());
    }

    public int readInt() {
        return byteBuffer.getInt();
    }

    public long readLong() {
        return byteBuffer.getLong();
    }

    public float readFloat32() {
        return byteBuffer.getFloat();
    }

    public double readFloat64() {
        return byteBuffer.getDouble();
    }

    public String readString() {
        final int size = readByte();
        byte[] bytes = new byte[size];
        byteBuffer.get(bytes);
        return new String(bytes, StandardCharsets.UTF_8);
    }

    public String readNullTerminatedString() {
        int position = byteBuffer.position();
        int size = 0;
        while (byteBuffer.hasRemaining()) {
            if (byteBuffer.get() == 0)
                break;
            size++;
        }
        byteBuffer.position(position);
        byte[] bytes = new byte[size];
        byteBuffer.get(bytes);
        byteBuffer.get(); // discard null terminator
        return new String(bytes, StandardCharsets.UTF_8);
    }

    public byte[] readBytes(int size) {
        byte[] bytes = new byte[size];

        byteBuffer.get(bytes);

        return bytes;
    }
}
