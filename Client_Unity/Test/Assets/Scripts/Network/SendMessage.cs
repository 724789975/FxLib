///<Remark>PanYangzhong，创建日期:2016.11.17</ Remark >
///发送消息封装类

using System.IO;

namespace Net
{
    public class SendMessage
    {
        uint _id;
        BinaryWriter _writer;
        BinaryWriter _dataWriter;

        public SendMessage(uint id)
        {
            _id = id;
            _writer = new BinaryWriter(new MemoryStream());
            _dataWriter = new BinaryWriter(new MemoryStream());
        }

        private void _writeHead()
        {
            MemoryStream stream = _dataWriter.BaseStream as MemoryStream;
            _writer.Write((uint)(stream.Length+4));
            _writer.Write(_id);
            byte[] data = stream.ToArray();
            _writer.Write(data);
        }

        public void Write<T>(T proto) where T : CBaseMsg
        {
            proto.Write(_dataWriter);
            _writeHead();
        }

        public byte[] GetBuffer()
        {
            MemoryStream stream = _writer.BaseStream as MemoryStream;
            return stream.ToArray();
        }
    }
}