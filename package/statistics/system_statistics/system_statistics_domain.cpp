
#include <wfc/iinterface.hpp>
#include "system_statistics_domain.hpp"
#include "impl/get_procstat.hpp"
namespace wfc{ namespace core{

struct protostat
{
  typedef system_statistics_domain::meter_ptr meter_ptr;
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
  auto prefix = this->options().prefix;
  auto proto = std::make_shared<protostat>();
  proto->utime = this->create_meter_prototype("", prefix + "utime");
  proto->stime = this->create_meter_prototype("", prefix + "stime");
  proto->cutime = this->create_meter_prototype("", prefix + "cutime");
  proto->cstime = this->create_meter_prototype("", prefix + "cstime");
  proto->vsize = this->create_meter_prototype("", prefix + "vsize");
  proto->rss= this->create_meter_prototype("", prefix + "rss");
  if ( proto->utime == nullptr )
    proto = nullptr;

  this->get_workflow()->release_timer(_timer_id);
  _timer_id = -1;
  if ( proto == nullptr )
    return;
  
  _timer_id = this->get_workflow()->create_timer( 
    std::chrono::milliseconds( this->options().interval_ms ),
    [this, proto]()->bool
    {
      procstat ps;
      if ( 0==get_procstat(&ps) )
      {
        this->create_meter(proto->utime,  0, ps.utime - proto->ps.utime );
        this->create_meter(proto->stime,  0, ps.stime - proto->ps.stime );
        this->create_meter(proto->cutime, 0, ps.cutime - proto->ps.cutime );
        this->create_meter(proto->cstime, 0, ps.cstime - proto->ps.cstime );
        this->create_meter(proto->vsize,  ps.vsize, 0);
        this->create_meter(proto->rss,  ps.rss, 0);
        proto->ps = ps;
      }
      return true;
    }
  );
  
}

}}
