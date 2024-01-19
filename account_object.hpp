#ifndef INCLUDE_ACCOUNT_OBJECT_HPP_
#define INCLUDE_ACCOUNT_OBJECT_HPP_

#include <chainbase/chainbase.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <string>
#include <iostream>

#include "name.hpp"
#include "types.hpp"
#include "permission_object.hpp"
#include "utils.hpp"

struct account_object : public chainbase::object<account_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   account_object(Constructor &&c, Allocator &&a) : abi(a)
   {
      c(*this);
   }

   id_type id;
   name account_name;
   fc::time_point creation_date;
   shared_blob abi;
};
using account_id_type = account_object::id_type;
struct by_id;
struct by_name;
using account_index = chainbase::shared_multi_index_container<
    account_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
        ordered_unique<tag<by_name>, member<account_object, name, &account_object::account_name>>>>;

struct account_metadata_object : public chainbase::object<account_metadata_object_type, int64_t>
{

   template <typename Constructor, typename Allocator>
   account_metadata_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }
   id_type id;
   name account_name; //< name should not be changed within a chainbase modifier lambda
   uint64_t recv_sequence = 0;
   uint64_t auth_sequence = 0;
   uint64_t code_sequence = 0;
   uint64_t abi_sequence = 0;
   digest_type code_hash;
   time_point last_code_update;
   uint32_t flags = 0;
   uint8_t vm_type = 0;
   uint8_t vm_version = 0;
};

struct by_name;
using account_metadata_index = chainbase::shared_multi_index_container<
    account_metadata_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<account_metadata_object, account_metadata_object::id_type, &account_metadata_object::id>>,
        ordered_unique<tag<by_name>, member<account_metadata_object, name, &account_metadata_object::account_name>>>>;

struct account_ram_correction_object : public chainbase::object<account_ram_correction_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   account_ram_correction_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }
   id_type id;
   name account_name; //< name should not be changed within a chainbase modifier lambda
   uint64_t ram_correction = 0;
};

struct by_name;
using account_ram_correction_index = chainbase::shared_multi_index_container<
    account_ram_correction_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<account_ram_correction_object, account_ram_correction_object::id_type, &account_ram_correction_object::id>>,
        ordered_unique<tag<by_name>, member<account_ram_correction_object, name, &account_ram_correction_object::account_name>>>>;

struct resource_limits_object : public chainbase::object<resource_limits_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   resource_limits_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   name owner;   //< owner should not be changed within a chainbase modifier lambda
   bool pending = false; //< pending should not be changed within a chainbase modifier lambda

   int64_t net_weight = -1;
   int64_t cpu_weight = -1;
   int64_t ram_bytes = -1;
};

struct by_owner;
struct by_dirty;

using resource_limits_index = chainbase::shared_multi_index_container<
    resource_limits_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<resource_limits_object, resource_limits_object::id_type, &resource_limits_object::id>>,
        ordered_unique<tag<by_owner>,
                       composite_key<resource_limits_object,
                                     BOOST_MULTI_INDEX_MEMBER(resource_limits_object, bool, pending),
                                     BOOST_MULTI_INDEX_MEMBER(resource_limits_object, name, owner)>>>>;

struct usage_accumulator
{
   usage_accumulator()
       : last_ordinal(0), value_ex(0), consumed(0)
   {
   }

   uint32_t last_ordinal; ///< The ordinal of the last period which has contributed to the average
   uint64_t value_ex;     ///< The current average pre-multiplied by Precision
   uint64_t consumed;     ///< The last periods average + the current periods contribution so far
};
struct resource_usage_object : public chainbase::object<resource_usage_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   resource_usage_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   account_name owner; //< owner should not be changed within a chainbase modifier lambda

   usage_accumulator net_usage;
   usage_accumulator cpu_usage;

   uint64_t ram_usage = 0;
};

using resource_usage_index = chainbase::shared_multi_index_container<
    resource_usage_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
        ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner>>>>;

CHAINBASE_SET_INDEX_TYPE(account_object, account_index)
CHAINBASE_SET_INDEX_TYPE(account_metadata_object, account_metadata_index)
CHAINBASE_SET_INDEX_TYPE(account_ram_correction_object, account_ram_correction_index)
CHAINBASE_SET_INDEX_TYPE(resource_limits_object, resource_limits_index)
CHAINBASE_SET_INDEX_TYPE(resource_usage_object, resource_usage_index)


void initialize_account(chainbase::database &db, const account_name& account_name) {
   const auto& limits = db.create<resource_limits_object>([&]( resource_limits_object& bl ) {
      bl.id = account_name.to_uint64_t();  
      bl.owner = account_name;
   });

   const auto& usage = db.create<resource_usage_object>([&]( resource_usage_object& bu ) {
      bu.id = account_name.to_uint64_t();  
      bu.owner = account_name;
   });
}

void account_add_indexes(chainbase::database &db)
{
   db.add_index<account_index>();
   db.add_index<account_metadata_index>();
   db.add_index<permission_index>();
   db.add_index<permission_usage_index>();
   db.add_index<resource_limits_index>();
   db.add_index<resource_usage_index>();
}

void create_native_account(chainbase::database &db, boost::filesystem::path path, name account_name)
{
   auto initDBSize = getCurrentSize(db, path);
   auto beforeDBSize = initDBSize;

   db.create<account_object>([&](auto &a)
                             {
                              a.id = account_name.to_uint64_t();
                              a.account_name = account_name; });
   auto afterDBSize = getCurrentSize(db, path);
   std::cout << "account_object :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   db.create<account_metadata_object>([&](auto &a)
                                      { 
                                       a.id = account_name.to_uint64_t();
                                       a.account_name = account_name; });
   afterDBSize = getCurrentSize(db, path);
   std::cout << "account_metadata_object :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   const auto& limits = db.create<resource_limits_object>([&]( resource_limits_object& bl ) {
      bl.id = account_name.to_uint64_t();  
      bl.owner = account_name;
   });
   afterDBSize = getCurrentSize(db, path);
   std::cout << "resource_limits_object :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   const auto& usage = db.create<resource_usage_object>([&]( resource_usage_object& bu ) {
      bu.id = account_name.to_uint64_t();  
      bu.owner = account_name;
   });
   afterDBSize = getCurrentSize(db, path);
   std::cout << "resource_usage_object :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   //  create permisison
   const auto &owner_permission = create_permission(db, account_name, name('owner'), 0);
   afterDBSize = getCurrentSize(db, path);
   std::cout << "owner_permission :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   const auto &active_permission = create_permission(db, account_name, name('active'), owner_permission.id);
   afterDBSize = getCurrentSize(db, path);
   std::cout << "active_permission :" << afterDBSize - beforeDBSize << std::endl;
   std::cout << "total size  :" << afterDBSize - initDBSize << std::endl;
}

#endif // INCLUDE_ACCOUNT_OBJECT_HPP_"