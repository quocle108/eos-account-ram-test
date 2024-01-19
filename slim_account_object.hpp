#ifndef INCLUDE_SLIM_ACCOUNT_OBJECT_HPP_
#define INCLUDE_SLIM_ACCOUNT_OBJECT_HPP_

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
#include "slim_permission_object.hpp"
struct slim_account_object : public chainbase::object<slim_account_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   slim_account_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   name account_name;
   fc::time_point creation_date;
   uint64_t recv_sequence = 0;
   uint64_t auth_sequence = 0;
};
using account_id_type = slim_account_object::id_type;
struct by_id;
using slim_account_index = chainbase::shared_multi_index_container<
    slim_account_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<slim_account_object, slim_account_object::id_type, &slim_account_object::id>>,
        ordered_unique<tag<by_name>, member<slim_account_object, name, &slim_account_object::account_name>>
      >>;


struct slim_resource_object : public chainbase::object<slim_resource_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   slim_resource_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   name owner;

   int64_t net_weight = -1;
   int64_t cpu_weight = -1;
   int64_t ram_bytes = -1;

   usage_accumulator net_usage;
   usage_accumulator cpu_usage;

   uint64_t ram_usage = 0;
};

struct by_owner;
struct by_dirty;

using slim_resource_index = chainbase::shared_multi_index_container<
    slim_resource_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<slim_resource_object, slim_resource_object::id_type, &slim_resource_object::id>>,
        ordered_unique<tag<by_name>, member<slim_resource_object, name, &slim_resource_object::owner>>>>;

CHAINBASE_SET_INDEX_TYPE(slim_account_object, slim_account_index)
CHAINBASE_SET_INDEX_TYPE(slim_resource_object, slim_resource_index)

void slim_account_add_indexes(chainbase::database &db)
{
   db.add_index<slim_account_index>();
   db.add_index<slim_resource_index>();
   db.add_index<slim_permission_index>();
}


void create_slim_account(chainbase::database &db, boost::filesystem::path path, name account_name)
{
   auto initDBSize = getCurrentSize(db, path);
   auto beforeDBSize = initDBSize;
   db.create<slim_account_object>([&](auto &a)
                             {
                              a.id = account_name.to_uint64_t();
                              a.account_name = account_name;
                              });
   auto afterDBSize = getCurrentSize(db, path);
   std::cout << "slim_account_object :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   const auto& limits = db.create<slim_resource_object>([&]( slim_resource_object& bl ) {
      bl.id = account_name.to_uint64_t();  
      bl.owner = account_name; 
   });
   afterDBSize = getCurrentSize(db, path);
   std::cout << "slim_resource_object :" << afterDBSize - beforeDBSize << std::endl;
   beforeDBSize = afterDBSize;

   // create permisison
    const auto& active_permission  = create_slim_permission(db, account_name, name('active'), 0 );
   afterDBSize = getCurrentSize(db, path);
   std::cout << "active_permission :" << afterDBSize - beforeDBSize << std::endl;
   std::cout << "total size  :" << afterDBSize - initDBSize << std::endl;
}

#endif  // INCLUDE_SLIM_ACCOUNT_OBJECT_HPP_"