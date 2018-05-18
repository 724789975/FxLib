// <auto-generated>
//     Generated by the protocol buffer compiler.  DO NOT EDIT!
//     source: web_data.proto
// </auto-generated>
#pragma warning disable 1591, 0612, 3021
#region Designer generated code

using pb = global::Google.Protobuf;
using pbc = global::Google.Protobuf.Collections;
using pbr = global::Google.Protobuf.Reflection;
using scg = global::System.Collections.Generic;
namespace GameProto {

  /// <summary>Holder for reflection information generated from web_data.proto</summary>
  public static partial class WebDataReflection {

    #region Descriptor
    /// <summary>File descriptor for web_data.proto</summary>
    public static pbr::FileDescriptor Descriptor {
      get { return descriptor; }
    }
    private static pbr::FileDescriptor descriptor;

    static WebDataReflection() {
      byte[] descriptorData = global::System.Convert.FromBase64String(
          string.Concat(
            "Cg53ZWJfZGF0YS5wcm90bxIJR2FtZVByb3RvIlkKCFJvbGVEYXRhEhQKDHF3",
            "X3BsYXllcl9pZBgBIAEoBBIUCgxzel9uaWNrX25hbWUYAiABKAkSEQoJc3pf",
            "YXZhdGFyGAMgASgJEg4KBmR3X3NleBgEIAEoDSJgCgxUZWFtUm9sZURhdGES",
            "JgoJcm9sZV9kYXRhGAEgASgLMhMuR2FtZVByb3RvLlJvbGVEYXRhEhIKCmR3",
            "X3Nsb3RfaWQYAiABKA0SFAoMZHdfc2VydmVyX2lkGAMgASgNIk4KDk9ubGlu",
            "ZVJvbGVEYXRhEiYKCXJvbGVfZGF0YRgBIAEoCzITLkdhbWVQcm90by5Sb2xl",
            "RGF0YRIUCgxkd19zZXJ2ZXJfaWQYAiABKA0iPwoOR2FtZUNvbmZpZ0Jhc2US",
            "FAoMZHdfZ2FtZV90eXBlGAEgASgNEhcKD2R3X3ByZXBhcmVfdGltZRgCIAEo",
            "DSJCChBHYW1lQ29uZmlnQ29tbW9uEi4KC2Jhc2VfY29uZmlnGAEgASgLMhku",
            "R2FtZVByb3RvLkdhbWVDb25maWdCYXNlKicKCUVHYW1lVHlwZRILCgdHVF9O",
            "b25lEAASDQoJR1RfQ29tbW9uEAFiBnByb3RvMw=="));
      descriptor = pbr::FileDescriptor.FromGeneratedCode(descriptorData,
          new pbr::FileDescriptor[] { },
          new pbr::GeneratedClrTypeInfo(new[] {typeof(global::GameProto.EGameType), }, new pbr::GeneratedClrTypeInfo[] {
            new pbr::GeneratedClrTypeInfo(typeof(global::GameProto.RoleData), global::GameProto.RoleData.Parser, new[]{ "QwPlayerId", "SzNickName", "SzAvatar", "DwSex" }, null, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::GameProto.TeamRoleData), global::GameProto.TeamRoleData.Parser, new[]{ "RoleData", "DwSlotId", "DwServerId" }, null, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::GameProto.OnlineRoleData), global::GameProto.OnlineRoleData.Parser, new[]{ "RoleData", "DwServerId" }, null, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::GameProto.GameConfigBase), global::GameProto.GameConfigBase.Parser, new[]{ "DwGameType", "DwPrepareTime" }, null, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::GameProto.GameConfigCommon), global::GameProto.GameConfigCommon.Parser, new[]{ "BaseConfig" }, null, null, null)
          }));
    }
    #endregion

  }
  #region Enums
  public enum EGameType {
    [pbr::OriginalName("GT_None")] GtNone = 0,
    [pbr::OriginalName("GT_Common")] GtCommon = 1,
  }

  #endregion

  #region Messages
  public sealed partial class RoleData : pb::IMessage<RoleData> {
    private static readonly pb::MessageParser<RoleData> _parser = new pb::MessageParser<RoleData>(() => new RoleData());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pb::MessageParser<RoleData> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::GameProto.WebDataReflection.Descriptor.MessageTypes[0]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public RoleData() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public RoleData(RoleData other) : this() {
      qwPlayerId_ = other.qwPlayerId_;
      szNickName_ = other.szNickName_;
      szAvatar_ = other.szAvatar_;
      dwSex_ = other.dwSex_;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public RoleData Clone() {
      return new RoleData(this);
    }

    /// <summary>Field number for the "qw_player_id" field.</summary>
    public const int QwPlayerIdFieldNumber = 1;
    private ulong qwPlayerId_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public ulong QwPlayerId {
      get { return qwPlayerId_; }
      set {
        qwPlayerId_ = value;
      }
    }

    /// <summary>Field number for the "sz_nick_name" field.</summary>
    public const int SzNickNameFieldNumber = 2;
    private string szNickName_ = "";
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public string SzNickName {
      get { return szNickName_; }
      set {
        szNickName_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "sz_avatar" field.</summary>
    public const int SzAvatarFieldNumber = 3;
    private string szAvatar_ = "";
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public string SzAvatar {
      get { return szAvatar_; }
      set {
        szAvatar_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "dw_sex" field.</summary>
    public const int DwSexFieldNumber = 4;
    private uint dwSex_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public uint DwSex {
      get { return dwSex_; }
      set {
        dwSex_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override bool Equals(object other) {
      return Equals(other as RoleData);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public bool Equals(RoleData other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (QwPlayerId != other.QwPlayerId) return false;
      if (SzNickName != other.SzNickName) return false;
      if (SzAvatar != other.SzAvatar) return false;
      if (DwSex != other.DwSex) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override int GetHashCode() {
      int hash = 1;
      if (QwPlayerId != 0UL) hash ^= QwPlayerId.GetHashCode();
      if (SzNickName.Length != 0) hash ^= SzNickName.GetHashCode();
      if (SzAvatar.Length != 0) hash ^= SzAvatar.GetHashCode();
      if (DwSex != 0) hash ^= DwSex.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void WriteTo(pb::CodedOutputStream output) {
      if (QwPlayerId != 0UL) {
        output.WriteRawTag(8);
        output.WriteUInt64(QwPlayerId);
      }
      if (SzNickName.Length != 0) {
        output.WriteRawTag(18);
        output.WriteString(SzNickName);
      }
      if (SzAvatar.Length != 0) {
        output.WriteRawTag(26);
        output.WriteString(SzAvatar);
      }
      if (DwSex != 0) {
        output.WriteRawTag(32);
        output.WriteUInt32(DwSex);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public int CalculateSize() {
      int size = 0;
      if (QwPlayerId != 0UL) {
        size += 1 + pb::CodedOutputStream.ComputeUInt64Size(QwPlayerId);
      }
      if (SzNickName.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(SzNickName);
      }
      if (SzAvatar.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(SzAvatar);
      }
      if (DwSex != 0) {
        size += 1 + pb::CodedOutputStream.ComputeUInt32Size(DwSex);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(RoleData other) {
      if (other == null) {
        return;
      }
      if (other.QwPlayerId != 0UL) {
        QwPlayerId = other.QwPlayerId;
      }
      if (other.SzNickName.Length != 0) {
        SzNickName = other.SzNickName;
      }
      if (other.SzAvatar.Length != 0) {
        SzAvatar = other.SzAvatar;
      }
      if (other.DwSex != 0) {
        DwSex = other.DwSex;
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(pb::CodedInputStream input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 8: {
            QwPlayerId = input.ReadUInt64();
            break;
          }
          case 18: {
            SzNickName = input.ReadString();
            break;
          }
          case 26: {
            SzAvatar = input.ReadString();
            break;
          }
          case 32: {
            DwSex = input.ReadUInt32();
            break;
          }
        }
      }
    }

  }

  public sealed partial class TeamRoleData : pb::IMessage<TeamRoleData> {
    private static readonly pb::MessageParser<TeamRoleData> _parser = new pb::MessageParser<TeamRoleData>(() => new TeamRoleData());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pb::MessageParser<TeamRoleData> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::GameProto.WebDataReflection.Descriptor.MessageTypes[1]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public TeamRoleData() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public TeamRoleData(TeamRoleData other) : this() {
      RoleData = other.roleData_ != null ? other.RoleData.Clone() : null;
      dwSlotId_ = other.dwSlotId_;
      dwServerId_ = other.dwServerId_;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public TeamRoleData Clone() {
      return new TeamRoleData(this);
    }

    /// <summary>Field number for the "role_data" field.</summary>
    public const int RoleDataFieldNumber = 1;
    private global::GameProto.RoleData roleData_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public global::GameProto.RoleData RoleData {
      get { return roleData_; }
      set {
        roleData_ = value;
      }
    }

    /// <summary>Field number for the "dw_slot_id" field.</summary>
    public const int DwSlotIdFieldNumber = 2;
    private uint dwSlotId_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public uint DwSlotId {
      get { return dwSlotId_; }
      set {
        dwSlotId_ = value;
      }
    }

    /// <summary>Field number for the "dw_server_id" field.</summary>
    public const int DwServerIdFieldNumber = 3;
    private uint dwServerId_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public uint DwServerId {
      get { return dwServerId_; }
      set {
        dwServerId_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override bool Equals(object other) {
      return Equals(other as TeamRoleData);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public bool Equals(TeamRoleData other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (!object.Equals(RoleData, other.RoleData)) return false;
      if (DwSlotId != other.DwSlotId) return false;
      if (DwServerId != other.DwServerId) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override int GetHashCode() {
      int hash = 1;
      if (roleData_ != null) hash ^= RoleData.GetHashCode();
      if (DwSlotId != 0) hash ^= DwSlotId.GetHashCode();
      if (DwServerId != 0) hash ^= DwServerId.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void WriteTo(pb::CodedOutputStream output) {
      if (roleData_ != null) {
        output.WriteRawTag(10);
        output.WriteMessage(RoleData);
      }
      if (DwSlotId != 0) {
        output.WriteRawTag(16);
        output.WriteUInt32(DwSlotId);
      }
      if (DwServerId != 0) {
        output.WriteRawTag(24);
        output.WriteUInt32(DwServerId);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public int CalculateSize() {
      int size = 0;
      if (roleData_ != null) {
        size += 1 + pb::CodedOutputStream.ComputeMessageSize(RoleData);
      }
      if (DwSlotId != 0) {
        size += 1 + pb::CodedOutputStream.ComputeUInt32Size(DwSlotId);
      }
      if (DwServerId != 0) {
        size += 1 + pb::CodedOutputStream.ComputeUInt32Size(DwServerId);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(TeamRoleData other) {
      if (other == null) {
        return;
      }
      if (other.roleData_ != null) {
        if (roleData_ == null) {
          roleData_ = new global::GameProto.RoleData();
        }
        RoleData.MergeFrom(other.RoleData);
      }
      if (other.DwSlotId != 0) {
        DwSlotId = other.DwSlotId;
      }
      if (other.DwServerId != 0) {
        DwServerId = other.DwServerId;
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(pb::CodedInputStream input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 10: {
            if (roleData_ == null) {
              roleData_ = new global::GameProto.RoleData();
            }
            input.ReadMessage(roleData_);
            break;
          }
          case 16: {
            DwSlotId = input.ReadUInt32();
            break;
          }
          case 24: {
            DwServerId = input.ReadUInt32();
            break;
          }
        }
      }
    }

  }

  public sealed partial class OnlineRoleData : pb::IMessage<OnlineRoleData> {
    private static readonly pb::MessageParser<OnlineRoleData> _parser = new pb::MessageParser<OnlineRoleData>(() => new OnlineRoleData());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pb::MessageParser<OnlineRoleData> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::GameProto.WebDataReflection.Descriptor.MessageTypes[2]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public OnlineRoleData() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public OnlineRoleData(OnlineRoleData other) : this() {
      RoleData = other.roleData_ != null ? other.RoleData.Clone() : null;
      dwServerId_ = other.dwServerId_;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public OnlineRoleData Clone() {
      return new OnlineRoleData(this);
    }

    /// <summary>Field number for the "role_data" field.</summary>
    public const int RoleDataFieldNumber = 1;
    private global::GameProto.RoleData roleData_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public global::GameProto.RoleData RoleData {
      get { return roleData_; }
      set {
        roleData_ = value;
      }
    }

    /// <summary>Field number for the "dw_server_id" field.</summary>
    public const int DwServerIdFieldNumber = 2;
    private uint dwServerId_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public uint DwServerId {
      get { return dwServerId_; }
      set {
        dwServerId_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override bool Equals(object other) {
      return Equals(other as OnlineRoleData);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public bool Equals(OnlineRoleData other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (!object.Equals(RoleData, other.RoleData)) return false;
      if (DwServerId != other.DwServerId) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override int GetHashCode() {
      int hash = 1;
      if (roleData_ != null) hash ^= RoleData.GetHashCode();
      if (DwServerId != 0) hash ^= DwServerId.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void WriteTo(pb::CodedOutputStream output) {
      if (roleData_ != null) {
        output.WriteRawTag(10);
        output.WriteMessage(RoleData);
      }
      if (DwServerId != 0) {
        output.WriteRawTag(16);
        output.WriteUInt32(DwServerId);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public int CalculateSize() {
      int size = 0;
      if (roleData_ != null) {
        size += 1 + pb::CodedOutputStream.ComputeMessageSize(RoleData);
      }
      if (DwServerId != 0) {
        size += 1 + pb::CodedOutputStream.ComputeUInt32Size(DwServerId);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(OnlineRoleData other) {
      if (other == null) {
        return;
      }
      if (other.roleData_ != null) {
        if (roleData_ == null) {
          roleData_ = new global::GameProto.RoleData();
        }
        RoleData.MergeFrom(other.RoleData);
      }
      if (other.DwServerId != 0) {
        DwServerId = other.DwServerId;
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(pb::CodedInputStream input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 10: {
            if (roleData_ == null) {
              roleData_ = new global::GameProto.RoleData();
            }
            input.ReadMessage(roleData_);
            break;
          }
          case 16: {
            DwServerId = input.ReadUInt32();
            break;
          }
        }
      }
    }

  }

  public sealed partial class GameConfigBase : pb::IMessage<GameConfigBase> {
    private static readonly pb::MessageParser<GameConfigBase> _parser = new pb::MessageParser<GameConfigBase>(() => new GameConfigBase());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pb::MessageParser<GameConfigBase> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::GameProto.WebDataReflection.Descriptor.MessageTypes[3]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public GameConfigBase() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public GameConfigBase(GameConfigBase other) : this() {
      dwGameType_ = other.dwGameType_;
      dwPrepareTime_ = other.dwPrepareTime_;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public GameConfigBase Clone() {
      return new GameConfigBase(this);
    }

    /// <summary>Field number for the "dw_game_type" field.</summary>
    public const int DwGameTypeFieldNumber = 1;
    private uint dwGameType_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public uint DwGameType {
      get { return dwGameType_; }
      set {
        dwGameType_ = value;
      }
    }

    /// <summary>Field number for the "dw_prepare_time" field.</summary>
    public const int DwPrepareTimeFieldNumber = 2;
    private uint dwPrepareTime_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public uint DwPrepareTime {
      get { return dwPrepareTime_; }
      set {
        dwPrepareTime_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override bool Equals(object other) {
      return Equals(other as GameConfigBase);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public bool Equals(GameConfigBase other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (DwGameType != other.DwGameType) return false;
      if (DwPrepareTime != other.DwPrepareTime) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override int GetHashCode() {
      int hash = 1;
      if (DwGameType != 0) hash ^= DwGameType.GetHashCode();
      if (DwPrepareTime != 0) hash ^= DwPrepareTime.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void WriteTo(pb::CodedOutputStream output) {
      if (DwGameType != 0) {
        output.WriteRawTag(8);
        output.WriteUInt32(DwGameType);
      }
      if (DwPrepareTime != 0) {
        output.WriteRawTag(16);
        output.WriteUInt32(DwPrepareTime);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public int CalculateSize() {
      int size = 0;
      if (DwGameType != 0) {
        size += 1 + pb::CodedOutputStream.ComputeUInt32Size(DwGameType);
      }
      if (DwPrepareTime != 0) {
        size += 1 + pb::CodedOutputStream.ComputeUInt32Size(DwPrepareTime);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(GameConfigBase other) {
      if (other == null) {
        return;
      }
      if (other.DwGameType != 0) {
        DwGameType = other.DwGameType;
      }
      if (other.DwPrepareTime != 0) {
        DwPrepareTime = other.DwPrepareTime;
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(pb::CodedInputStream input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 8: {
            DwGameType = input.ReadUInt32();
            break;
          }
          case 16: {
            DwPrepareTime = input.ReadUInt32();
            break;
          }
        }
      }
    }

  }

  public sealed partial class GameConfigCommon : pb::IMessage<GameConfigCommon> {
    private static readonly pb::MessageParser<GameConfigCommon> _parser = new pb::MessageParser<GameConfigCommon>(() => new GameConfigCommon());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pb::MessageParser<GameConfigCommon> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::GameProto.WebDataReflection.Descriptor.MessageTypes[4]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public GameConfigCommon() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public GameConfigCommon(GameConfigCommon other) : this() {
      BaseConfig = other.baseConfig_ != null ? other.BaseConfig.Clone() : null;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public GameConfigCommon Clone() {
      return new GameConfigCommon(this);
    }

    /// <summary>Field number for the "base_config" field.</summary>
    public const int BaseConfigFieldNumber = 1;
    private global::GameProto.GameConfigBase baseConfig_;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public global::GameProto.GameConfigBase BaseConfig {
      get { return baseConfig_; }
      set {
        baseConfig_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override bool Equals(object other) {
      return Equals(other as GameConfigCommon);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public bool Equals(GameConfigCommon other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (!object.Equals(BaseConfig, other.BaseConfig)) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override int GetHashCode() {
      int hash = 1;
      if (baseConfig_ != null) hash ^= BaseConfig.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void WriteTo(pb::CodedOutputStream output) {
      if (baseConfig_ != null) {
        output.WriteRawTag(10);
        output.WriteMessage(BaseConfig);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public int CalculateSize() {
      int size = 0;
      if (baseConfig_ != null) {
        size += 1 + pb::CodedOutputStream.ComputeMessageSize(BaseConfig);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(GameConfigCommon other) {
      if (other == null) {
        return;
      }
      if (other.baseConfig_ != null) {
        if (baseConfig_ == null) {
          baseConfig_ = new global::GameProto.GameConfigBase();
        }
        BaseConfig.MergeFrom(other.BaseConfig);
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    public void MergeFrom(pb::CodedInputStream input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 10: {
            if (baseConfig_ == null) {
              baseConfig_ = new global::GameProto.GameConfigBase();
            }
            input.ReadMessage(baseConfig_);
            break;
          }
        }
      }
    }

  }

  #endregion

}

#endregion Designer generated code
