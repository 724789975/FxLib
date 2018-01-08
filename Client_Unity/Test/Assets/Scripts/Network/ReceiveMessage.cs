///<Remark>PanYangzhong，创建日期:2016.10.1</ Remark >
///
using System;
using System.IO;

namespace Net
{
    public class ReceiveMessage
    {
        public BinaryReader _reader;
        uint _id;
        ushort _length;
        CBaseMsg _data;

        public ReceiveMessage(byte[] buffer, int length)
        {
            MemoryStream stream = new MemoryStream(buffer, 0, length);
            _reader = new BinaryReader(stream);
        }

        public uint GetID()
        {
            return _id;
        }

        public void ReadHead()
        {
            //_length = _reader.ReadUInt16();
            _id = _reader.ReadUInt32();
        }

        public T Read<T>() where T : CBaseMsg
        {
            if (_data == null)
            {
                T msg = Activator.CreateInstance<T>();
                msg.Read(_reader);
                _data = msg as CBaseMsg;
            }
            return (T)_data;
        }

    }

}