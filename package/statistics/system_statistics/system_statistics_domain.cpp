
#include <wfc/iinterface.hpp>
#include <wfc/statistics/meters.hpp>
#include "system_statistics_domain.hpp"
#include "impl/get_procstat.hpp"
namespace wfc{ namespace core{

struct protostat
{
  typedef ::wfc::value_meter meter_type;
  meter_type utime;                    /** user mode jiffies **/
  meter_type stime;                    /** kernel mode jiffies **/
  // meter_ptr cutime;                   /** user mode jiffies with childs **/
  // meter_ptr cstime;                   /** kernel mode jiffies with childs **/
  meter_type vsize;                    /** Virtual memory size **/
  meter_type rss;                      /** Resident Set Size **/
  int pid = 0;
  procstat ps;
};

class procmeter
{
  typedef protostat::meter_type meter_type;
  typedef std::mutex mutex_type;
public:
  procmeter(std::weak_ptr<wfc::statistics::statistics> stat,  const std::string& prefix)
    : _prefix(prefix)
    , _wstat(stat)
    , _procstat()
  {
  }

  void initialize(  )
  {
    std::lock_guard<mutex_type> lk(_mutext);
    _threads.clear();
    _procstat = this->create_protostat_("", -1);
    _procstat.pid = ::getpid();
  }

  void add_threads( std::string name, std::vector<pid_t> ids )
  {
    std::lock_guard<mutex_type> lk(_mutext);
    for (size_t i=0; i < ids.size(); ++i)
    {
      _threads.push_back( this->create_protostat_( name, static_cast<int>(i) ) );
      _threads.back().pid = ids[i];
    }
  }

  void perform()
  {
    std::lock_guard<mutex_type> lk(_mutext);
    this->perform_( &_procstat );
    for ( auto& t : _threads )
      this->perform_( &t );
  }

  
private:
  void perform_( protostat* proto )
  {
    if ( auto stat = _wstat.lock() )
    {
      procstat ps;
      if ( 0==get_procstat(proto->pid, &ps) )
      {
        if ( proto->ps.utime != 0 || true)
          proto->utime.create(0, ps.utime - proto->ps.utime);
        if ( proto->ps.stime != 0 )
          proto->stime.create(0, ps.stime - proto->ps.stime );
        /*if ( proto->ps.cutime != 0 )
          stat->create_meter(proto->cutime, 0, ps.cutime - proto->ps.cutime );
        if ( proto->ps.cstime != 0 )
          stat->create_meter(proto->cstime, 0, ps.cstime - proto->ps.cstime );
        */
        proto->vsize.create(ps.vsize*getpagesize()/*/(1024*1024)*/, 0);
        proto->rss.create(ps.rss*getpagesize()/*/(1024*1024)*/, 0);
        proto->ps = ps;
      }
    }
  }

  protostat create_protostat_(std::string name, int pid)
  {
    protostat proto;
    proto.utime = this->create_meter_(pid, name, "utime");
    proto.stime = this->create_meter_(pid, name, "stime");
    /*proto.cutime = this->create_meter_(pid, name, "cutime");
    proto.cstime = this->create_meter_(pid, name, "cstime");*/
    proto.vsize = this->create_meter_(pid, name, "vsize");
    proto.rss= this->create_meter_(pid, name, "rss");
    return proto;
  }

  value_meter create_meter_( int id, const std::string& group, const std::string& name)
  {
    if ( auto stat = _wstat.lock() )
    {
      std::stringstream ss;
      ss << _prefix << group;
      if ( id != -1 )
        ss << "thread" << id << ".";
      ss << name;
      return stat->create_value_meter(ss.str());
    }
    return value_meter();
  }
private:
  std::string _prefix;
  std::weak_ptr<wfc::statistics::statistics> _wstat;
  protostat _procstat;
  std::vector<protostat> _threads;
  mutex_type _mutext;
};
  
void system_statistics_domain::configure()
{

}

void system_statistics_domain::stop() 
{
  this->get_workflow()->release_timer(_timer_id);
  //this->get_workflow()->release_timer(_timer_id2);
}



void system_statistics_domain::restart()
{
  this->get_workflow()->release_timer(_timer_id);
  //this->get_workflow()->release_timer(_timer_id2);
  //_timer_id2 = 
  _timer_id = -1; 

  auto stat = this->get_statistics();
  if ( stat == nullptr )
    return;
  
  auto prefix = this->statistics_options().prefix;
  auto proto = std::make_shared<protostat>();
  proto->utime = stat->create_value_meter(prefix + "utime");
  proto->stime = stat->create_value_meter(prefix + "stime");
  /*proto->cutime = stat->create_value_prototype(prefix + "cutime");
  proto->cstime = stat->create_value_prototype(prefix + "cstime");*/
  proto->vsize = stat->create_value_meter(prefix + "vsize");
  proto->rss= stat->create_value_meter(prefix + "rss");
  
  _timer_id = this->get_workflow()->create_timer( 
    std::chrono::milliseconds( this->statistics_options().interval_ms ),
    [stat, proto, this]()->bool
    {
      procstat ps;
      if ( 0==get_procstat(&ps) )
      {
        if ( proto->ps.utime != 0 )
          proto->utime.create(0, ps.utime - proto->ps.utime );
        if ( proto->ps.stime != 0 )
          proto->stime.create(0, ps.stime - proto->ps.stime );
        /*if ( proto->ps.cutime != 0 )
          proto->cutime.create( 0, ps.cutime - proto->ps.cutime );*/
        /*if ( proto->ps.cstime != 0 )
          proto->cstime.create_meter( 0, ps.cstime - proto->ps.cstime );*/

        proto->vsize.create(ps.vsize/(1024*1024), 0);
        proto->rss.create(ps.rss*getpagesize()/(1024*1024), 0);
        proto->ps = ps;
      }
      return true;
    }
  );
  
  
}

}}
