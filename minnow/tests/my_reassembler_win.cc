#include "random.hh"
#include "reassembler_test_harness.hh"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

// static constexpr size_t NREPS = 4;
// static constexpr size_t NSEGS = 16;
// static constexpr size_t MAX_SEG_LEN = 128;
static constexpr size_t NREPS = 5000;
static constexpr size_t NSEGS = 16;
static constexpr size_t MAX_SEG_LEN = 512;
int main()
{
  // try {
  //   auto rd = get_random_engine();

  //   // overlapping segments
  //   ReassemblerTestHarness sr { "win test " + to_string( 0 ), NSEGS * MAX_SEG_LEN };

  //   vector<tuple<size_t, size_t>> seq_size = {{46, 205}, {577, 137}, {372, 866}, {684, 187}, {212, 225}, {171, 206}, {0, 598}, {0, 73}, {605, 321}, {0, 162}, {0, 865}, {278, 837}, {52, 723}, {240, 239}, {0, 277}, {372, 665}};

  //   string d( 1238, 0 );
  //   generate( d.begin(), d.end(), [&] { return rd(); } );

  //   for ( auto [off, sz] : seq_size ) {
  //   sr.execute( Insert { d.substr( off, sz ), off }.is_last( off + sz == 1238 ) );
  //   }

  //   sr.execute( ReadAll { d } );
  // } catch ( const exception& e ) {
  //   cerr << "Exception: " << e.what() << endl;
  //   return EXIT_FAILURE;
  // }

  // try {
  //     auto rd = get_random_engine();

  //     ReassemblerTestHarness sr { "win test " + to_string( 0 ), NSEGS * MAX_SEG_LEN };

  //     vector<tuple<size_t, size_t>> seq_size = {{1119, 295}, {0, 138}, {190, 381}, {0, 48}, {285, 696}, {0, 39}, {0, 882}, {0, 333}, {4, 249}, {708, 355}, {0, 696}, {328, 481}, {436, 1087}, {0, 457}, {539, 759}, {439, 740}};

  //     string d( 1523, 0 );
  //     generate( d.begin(), d.end(), [&] { return rd(); } );

  //     for ( auto [off, sz] : seq_size ) {
  //     sr.execute( Insert { d.substr( off, sz ), off }.is_last( off + sz == 1523 ) );
  //     }

  //     sr.execute( ReadAll { d } );

  // } catch ( const exception& e ) {
  //     cerr << "Exception: " << e.what() << endl;
  //     return EXIT_FAILURE;
  // }

  try {
      auto rd = get_random_engine();

      ReassemblerTestHarness sr { "win test " + to_string( 0 ), NSEGS * MAX_SEG_LEN };

      vector<tuple<size_t, size_t>> seq_size = {{3664, 768}, {3090, 1006}, {2019, 326}, {611, 394}, {1355, 247}, {1322, 459}, {2757, 403}, {3272, 1158}, {1521, 96}, {0, 748}, {0, 481}, {250, 1108}, {2690, 983}, {1525, 642}, {2628, 621}, {1848, 951}};

      string d( 4432, 0 );
      generate( d.begin(), d.end(), [&] { return rd(); } );

      for ( auto [off, sz] : seq_size ) {
      sr.execute( Insert { d.substr( off, sz ), off }.is_last( off + sz == 4432 ) );
      }

      sr.execute( ReadAll { d } );

  } catch ( const exception& e ) {
      cerr << "Exception: " << e.what() << endl;
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
