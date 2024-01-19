
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
#include <boost/interprocess/managed_external_buffer.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/io_service.hpp>

#include <cinttypes>
#include <inttypes.h>
#include "multi_index_includes.hpp"
#include "name.hpp"
#include "types.hpp"
#include "account_object.hpp"
#include "slim_account_object.hpp"
#include "permission_object.hpp"
#include "utils.hpp"

using namespace std;
using namespace chainbase;
using namespace boost::multi_index;
namespace bfs = boost::filesystem;


/**
    This simple program will open temp and add two new books every time
    it is run and then print out all of the books in the database.
 */
int main(int argc, char **argv)
{

   boost::filesystem::path temp = boost::filesystem::unique_path();
   chainbase::database db(temp, database::read_write, (uint64_t)1024 * 1024 * 1024 * 3);
   account_add_indexes(db);
   for (auto i = 0; i < 1; i++)
   {
      create_native_account(db, temp, name(i));
   }
   bfs::remove_all(temp);
   
   boost::filesystem::path temp1 = boost::filesystem::unique_path();
   chainbase::database db1(temp1, database::read_write, (uint64_t)1024 * 1024 * 1024 * 3 );
   slim_account_add_indexes(db1);
   for (auto i = 0; i < 1; i++)
   {
      create_slim_account(db1, temp1, name(i));
   }

   bfs::remove_all(temp1);
   return 0;
}
