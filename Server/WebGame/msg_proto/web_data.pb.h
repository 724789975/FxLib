// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: web_data.proto

#ifndef PROTOBUF_web_5fdata_2eproto__INCLUDED
#define PROTOBUF_web_5fdata_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3005001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace protobuf_web_5fdata_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[2];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
void InitDefaultsRoleDataImpl();
void InitDefaultsRoleData();
void InitDefaultsTeamRoleDataImpl();
void InitDefaultsTeamRoleData();
inline void InitDefaults() {
  InitDefaultsRoleData();
  InitDefaultsTeamRoleData();
}
}  // namespace protobuf_web_5fdata_2eproto
namespace GameProto {
class RoleData;
class RoleDataDefaultTypeInternal;
extern RoleDataDefaultTypeInternal _RoleData_default_instance_;
class TeamRoleData;
class TeamRoleDataDefaultTypeInternal;
extern TeamRoleDataDefaultTypeInternal _TeamRoleData_default_instance_;
}  // namespace GameProto
namespace GameProto {

// ===================================================================

class RoleData : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:GameProto.RoleData) */ {
 public:
  RoleData();
  virtual ~RoleData();

  RoleData(const RoleData& from);

  inline RoleData& operator=(const RoleData& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  RoleData(RoleData&& from) noexcept
    : RoleData() {
    *this = ::std::move(from);
  }

  inline RoleData& operator=(RoleData&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const RoleData& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const RoleData* internal_default_instance() {
    return reinterpret_cast<const RoleData*>(
               &_RoleData_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(RoleData* other);
  friend void swap(RoleData& a, RoleData& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline RoleData* New() const PROTOBUF_FINAL { return New(NULL); }

  RoleData* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const RoleData& from);
  void MergeFrom(const RoleData& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(RoleData* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string sz_nick_name = 2;
  void clear_sz_nick_name();
  static const int kSzNickNameFieldNumber = 2;
  const ::std::string& sz_nick_name() const;
  void set_sz_nick_name(const ::std::string& value);
  #if LANG_CXX11
  void set_sz_nick_name(::std::string&& value);
  #endif
  void set_sz_nick_name(const char* value);
  void set_sz_nick_name(const char* value, size_t size);
  ::std::string* mutable_sz_nick_name();
  ::std::string* release_sz_nick_name();
  void set_allocated_sz_nick_name(::std::string* sz_nick_name);

  // string sz_avatar = 3;
  void clear_sz_avatar();
  static const int kSzAvatarFieldNumber = 3;
  const ::std::string& sz_avatar() const;
  void set_sz_avatar(const ::std::string& value);
  #if LANG_CXX11
  void set_sz_avatar(::std::string&& value);
  #endif
  void set_sz_avatar(const char* value);
  void set_sz_avatar(const char* value, size_t size);
  ::std::string* mutable_sz_avatar();
  ::std::string* release_sz_avatar();
  void set_allocated_sz_avatar(::std::string* sz_avatar);

  // uint64 qw_player_id = 1;
  void clear_qw_player_id();
  static const int kQwPlayerIdFieldNumber = 1;
  ::google::protobuf::uint64 qw_player_id() const;
  void set_qw_player_id(::google::protobuf::uint64 value);

  // uint32 dw_sex = 4;
  void clear_dw_sex();
  static const int kDwSexFieldNumber = 4;
  ::google::protobuf::uint32 dw_sex() const;
  void set_dw_sex(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:GameProto.RoleData)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr sz_nick_name_;
  ::google::protobuf::internal::ArenaStringPtr sz_avatar_;
  ::google::protobuf::uint64 qw_player_id_;
  ::google::protobuf::uint32 dw_sex_;
  mutable int _cached_size_;
  friend struct ::protobuf_web_5fdata_2eproto::TableStruct;
  friend void ::protobuf_web_5fdata_2eproto::InitDefaultsRoleDataImpl();
};
// -------------------------------------------------------------------

class TeamRoleData : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:GameProto.TeamRoleData) */ {
 public:
  TeamRoleData();
  virtual ~TeamRoleData();

  TeamRoleData(const TeamRoleData& from);

  inline TeamRoleData& operator=(const TeamRoleData& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  TeamRoleData(TeamRoleData&& from) noexcept
    : TeamRoleData() {
    *this = ::std::move(from);
  }

  inline TeamRoleData& operator=(TeamRoleData&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const TeamRoleData& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const TeamRoleData* internal_default_instance() {
    return reinterpret_cast<const TeamRoleData*>(
               &_TeamRoleData_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(TeamRoleData* other);
  friend void swap(TeamRoleData& a, TeamRoleData& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline TeamRoleData* New() const PROTOBUF_FINAL { return New(NULL); }

  TeamRoleData* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const TeamRoleData& from);
  void MergeFrom(const TeamRoleData& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(TeamRoleData* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // .GameProto.RoleData role_data = 1;
  bool has_role_data() const;
  void clear_role_data();
  static const int kRoleDataFieldNumber = 1;
  const ::GameProto::RoleData& role_data() const;
  ::GameProto::RoleData* release_role_data();
  ::GameProto::RoleData* mutable_role_data();
  void set_allocated_role_data(::GameProto::RoleData* role_data);

  // uint32 dw_slot_id = 2;
  void clear_dw_slot_id();
  static const int kDwSlotIdFieldNumber = 2;
  ::google::protobuf::uint32 dw_slot_id() const;
  void set_dw_slot_id(::google::protobuf::uint32 value);

  // uint32 dw_server_id = 3;
  void clear_dw_server_id();
  static const int kDwServerIdFieldNumber = 3;
  ::google::protobuf::uint32 dw_server_id() const;
  void set_dw_server_id(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:GameProto.TeamRoleData)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::GameProto::RoleData* role_data_;
  ::google::protobuf::uint32 dw_slot_id_;
  ::google::protobuf::uint32 dw_server_id_;
  mutable int _cached_size_;
  friend struct ::protobuf_web_5fdata_2eproto::TableStruct;
  friend void ::protobuf_web_5fdata_2eproto::InitDefaultsTeamRoleDataImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// RoleData

// uint64 qw_player_id = 1;
inline void RoleData::clear_qw_player_id() {
  qw_player_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 RoleData::qw_player_id() const {
  // @@protoc_insertion_point(field_get:GameProto.RoleData.qw_player_id)
  return qw_player_id_;
}
inline void RoleData::set_qw_player_id(::google::protobuf::uint64 value) {
  
  qw_player_id_ = value;
  // @@protoc_insertion_point(field_set:GameProto.RoleData.qw_player_id)
}

// string sz_nick_name = 2;
inline void RoleData::clear_sz_nick_name() {
  sz_nick_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RoleData::sz_nick_name() const {
  // @@protoc_insertion_point(field_get:GameProto.RoleData.sz_nick_name)
  return sz_nick_name_.GetNoArena();
}
inline void RoleData::set_sz_nick_name(const ::std::string& value) {
  
  sz_nick_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:GameProto.RoleData.sz_nick_name)
}
#if LANG_CXX11
inline void RoleData::set_sz_nick_name(::std::string&& value) {
  
  sz_nick_name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:GameProto.RoleData.sz_nick_name)
}
#endif
inline void RoleData::set_sz_nick_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  sz_nick_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:GameProto.RoleData.sz_nick_name)
}
inline void RoleData::set_sz_nick_name(const char* value, size_t size) {
  
  sz_nick_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:GameProto.RoleData.sz_nick_name)
}
inline ::std::string* RoleData::mutable_sz_nick_name() {
  
  // @@protoc_insertion_point(field_mutable:GameProto.RoleData.sz_nick_name)
  return sz_nick_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RoleData::release_sz_nick_name() {
  // @@protoc_insertion_point(field_release:GameProto.RoleData.sz_nick_name)
  
  return sz_nick_name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RoleData::set_allocated_sz_nick_name(::std::string* sz_nick_name) {
  if (sz_nick_name != NULL) {
    
  } else {
    
  }
  sz_nick_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), sz_nick_name);
  // @@protoc_insertion_point(field_set_allocated:GameProto.RoleData.sz_nick_name)
}

// string sz_avatar = 3;
inline void RoleData::clear_sz_avatar() {
  sz_avatar_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RoleData::sz_avatar() const {
  // @@protoc_insertion_point(field_get:GameProto.RoleData.sz_avatar)
  return sz_avatar_.GetNoArena();
}
inline void RoleData::set_sz_avatar(const ::std::string& value) {
  
  sz_avatar_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:GameProto.RoleData.sz_avatar)
}
#if LANG_CXX11
inline void RoleData::set_sz_avatar(::std::string&& value) {
  
  sz_avatar_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:GameProto.RoleData.sz_avatar)
}
#endif
inline void RoleData::set_sz_avatar(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  sz_avatar_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:GameProto.RoleData.sz_avatar)
}
inline void RoleData::set_sz_avatar(const char* value, size_t size) {
  
  sz_avatar_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:GameProto.RoleData.sz_avatar)
}
inline ::std::string* RoleData::mutable_sz_avatar() {
  
  // @@protoc_insertion_point(field_mutable:GameProto.RoleData.sz_avatar)
  return sz_avatar_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RoleData::release_sz_avatar() {
  // @@protoc_insertion_point(field_release:GameProto.RoleData.sz_avatar)
  
  return sz_avatar_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RoleData::set_allocated_sz_avatar(::std::string* sz_avatar) {
  if (sz_avatar != NULL) {
    
  } else {
    
  }
  sz_avatar_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), sz_avatar);
  // @@protoc_insertion_point(field_set_allocated:GameProto.RoleData.sz_avatar)
}

// uint32 dw_sex = 4;
inline void RoleData::clear_dw_sex() {
  dw_sex_ = 0u;
}
inline ::google::protobuf::uint32 RoleData::dw_sex() const {
  // @@protoc_insertion_point(field_get:GameProto.RoleData.dw_sex)
  return dw_sex_;
}
inline void RoleData::set_dw_sex(::google::protobuf::uint32 value) {
  
  dw_sex_ = value;
  // @@protoc_insertion_point(field_set:GameProto.RoleData.dw_sex)
}

// -------------------------------------------------------------------

// TeamRoleData

// .GameProto.RoleData role_data = 1;
inline bool TeamRoleData::has_role_data() const {
  return this != internal_default_instance() && role_data_ != NULL;
}
inline void TeamRoleData::clear_role_data() {
  if (GetArenaNoVirtual() == NULL && role_data_ != NULL) {
    delete role_data_;
  }
  role_data_ = NULL;
}
inline const ::GameProto::RoleData& TeamRoleData::role_data() const {
  const ::GameProto::RoleData* p = role_data_;
  // @@protoc_insertion_point(field_get:GameProto.TeamRoleData.role_data)
  return p != NULL ? *p : *reinterpret_cast<const ::GameProto::RoleData*>(
      &::GameProto::_RoleData_default_instance_);
}
inline ::GameProto::RoleData* TeamRoleData::release_role_data() {
  // @@protoc_insertion_point(field_release:GameProto.TeamRoleData.role_data)
  
  ::GameProto::RoleData* temp = role_data_;
  role_data_ = NULL;
  return temp;
}
inline ::GameProto::RoleData* TeamRoleData::mutable_role_data() {
  
  if (role_data_ == NULL) {
    role_data_ = new ::GameProto::RoleData;
  }
  // @@protoc_insertion_point(field_mutable:GameProto.TeamRoleData.role_data)
  return role_data_;
}
inline void TeamRoleData::set_allocated_role_data(::GameProto::RoleData* role_data) {
  ::google::protobuf::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == NULL) {
    delete role_data_;
  }
  if (role_data) {
    ::google::protobuf::Arena* submessage_arena = NULL;
    if (message_arena != submessage_arena) {
      role_data = ::google::protobuf::internal::GetOwnedMessage(
          message_arena, role_data, submessage_arena);
    }
    
  } else {
    
  }
  role_data_ = role_data;
  // @@protoc_insertion_point(field_set_allocated:GameProto.TeamRoleData.role_data)
}

// uint32 dw_slot_id = 2;
inline void TeamRoleData::clear_dw_slot_id() {
  dw_slot_id_ = 0u;
}
inline ::google::protobuf::uint32 TeamRoleData::dw_slot_id() const {
  // @@protoc_insertion_point(field_get:GameProto.TeamRoleData.dw_slot_id)
  return dw_slot_id_;
}
inline void TeamRoleData::set_dw_slot_id(::google::protobuf::uint32 value) {
  
  dw_slot_id_ = value;
  // @@protoc_insertion_point(field_set:GameProto.TeamRoleData.dw_slot_id)
}

// uint32 dw_server_id = 3;
inline void TeamRoleData::clear_dw_server_id() {
  dw_server_id_ = 0u;
}
inline ::google::protobuf::uint32 TeamRoleData::dw_server_id() const {
  // @@protoc_insertion_point(field_get:GameProto.TeamRoleData.dw_server_id)
  return dw_server_id_;
}
inline void TeamRoleData::set_dw_server_id(::google::protobuf::uint32 value) {
  
  dw_server_id_ = value;
  // @@protoc_insertion_point(field_set:GameProto.TeamRoleData.dw_server_id)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace GameProto

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_web_5fdata_2eproto__INCLUDED
