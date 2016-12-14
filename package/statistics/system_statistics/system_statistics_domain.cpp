
#include <wfc/iinterface.hpp>
#include <wfc/statistics/meters.hpp>
#include "system_statistics_domain.hpp"
#include "impl/get_procstat.hpp"
namespace wfc{ namespace core{

struct protostat
{
  typedef std::shared_ptr< ::wfc::value_meter > meter_ptr;
  meter_ptr utime;                    /** user mode jiffies **/
  meter_ptr stime;                    /** kernel mode jiffies **/
  meter_ptr cutime;                   /** user mode jiffies with childs **/
  meter_ptr cstime;                   /** kernel mode jiffies with childs **/
  meter_ptr vsize;                    /** Virtual memory size **/
  meter_ptr rss;                      /** Resident Set Size **/

  procstat ps;
};
  
void system_statistics_domain::configure()
{

}

void system_statistics_domain::initialize()
{
  this->get_workflow()->release_timer(_timer_id);
  _timer_id = -1;

  auto stat = this->get_statistics();
  if ( stat == nullptr )
    return;
  
  auto prefix = this->options().prefix;
  auto proto = std::make_shared<protostat>();
  proto->utime = stat->create_value_prototype(prefix + "utime");
  proto->stime = stat->create_value_prototype(prefix + "stime");
  proto->cutime = stat->create_value_prototype(prefix + "cutime");
  proto->cstime = stat->create_value_prototype(prefix + "cstime");
  proto->vsize = stat->create_value_prototype(prefix + "vsize");
  proto->rss= stat->create_value_prototype(prefix + "rss");
  
  _timer_id = this->get_workflow()->create_timer( 
    std::chrono::milliseconds( this->options().interval_ms ),
    [stat, proto]()->bool
    {
      procstat ps;
      if ( 0==get_procstat(&ps) )
      {
        if ( proto->ps.utime != 0 )
          stat->create_meter(proto->utime,  0, ps.utime - proto->ps.utime );
        if ( proto->ps.stime != 0 )
          stat->create_meter(proto->stime,  0, ps.stime - proto->ps.stime );
        if ( proto->ps.cutime != 0 )
          stat->create_meter(proto->cutime, 0, ps.cutime - proto->ps.cutime );
        if ( proto->ps.cstime != 0 )
          stat->create_meter(proto->cstime, 0, ps.cstime - proto->ps.cstime );
        stat->create_meter(proto->vsize,  ps.vsize/(1024*1024), 0);
        stat->create_meter(proto->rss,  ps.rss*getpagesize()/(1024*1024), 0);
        proto->ps = ps;
      }
      return true;
    }
  );
  
}

}}
