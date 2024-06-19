#include <fas/testing.hpp>
#include <wfc/testing.hpp>
#include <package/prometheus/prometheus/prometheus_domain.hpp>

namespace {

  /*
GET /metrics HTTP/1.1
Host: example.com
User-Agent: Prometheus/2.22.1
Accept: text/plain
   */
UNIT(init, "")
{
  using namespace fas::testing;
  t << nothing;

#ifdef WITH_PROMETHEUS
  wfc::testing_domain td;
  wfc::core::prometheus_domain::domain_config conf;
  conf.name = "prometheus1";
  conf.addr = "0.0.0.0";
  conf.port = "30000";
  td.create<wfc::core::prometheus_domain>(conf);
  td.initialize();
  td.start();
  t << message("Listening...");
  td.global()->idle.insert( []() noexcept ->bool{ return true;});
  td.global()->io_context.run_for( std::chrono::seconds(1) );
#endif

}

}
  
BEGIN_SUITE(package_suite, "")
  ADD_UNIT( init )
END_SUITE(package_suite)
