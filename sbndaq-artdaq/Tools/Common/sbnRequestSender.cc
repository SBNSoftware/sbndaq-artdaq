#define TRACE_NAME "sbnRequestSender"

#include <boost/program_options.hpp>
#include "fhiclcpp/make_ParameterSet.h"
namespace bpo = boost::program_options;

#include "artdaq-core/Utilities/configureMessageFacility.hh"
#include "artdaq/Application/LoadParameterSet.hh"
#include "artdaq/DAQrate/RequestReceiver.hh"
#include "artdaq/DAQrate/RequestSender.hh"

int main(int argc, char* argv[]) {
  artdaq::configureMessageFacility("sbnRequestSender");

  struct Config {
    fhicl::TableFragment<artdaq::RequestSender::Config> senderConfig;
    fhicl::Atom<bool> use_receiver{fhicl::Name{"use_receiver"}, fhicl::Comment{"Whether to setup a RequestReceiver to verify that requests are being sent"}, false};
    fhicl::Atom<size_t> receiver_timeout_ms{fhicl::Name{"receiver_timeout_ms"}, fhicl::Comment{"Amount of time to wait for the receiver to receive a request message"}, 1000};
    fhicl::Table<artdaq::RequestReceiver::Config> receiver_config{fhicl::Name{"receiver_config"}, fhicl::Comment{"Configuration for RequestReceiver, if used"}};
    fhicl::Atom<int> num_requests{fhicl::Name{"num_requests"}, fhicl::Comment{"Number of requests to send"}};
    fhicl::Atom<artdaq::Fragment::sequence_id_t> starting_sequence_id{fhicl::Name{"starting_sequence_id"}, fhicl::Comment{"Sequence ID of first request"}, 1};
    fhicl::Atom<artdaq::Fragment::sequence_id_t> sequence_id_scale{fhicl::Name{"sequence_id_step"}, fhicl::Comment{"Amount to increment Sequence ID for each request"}, 1};
    fhicl::Atom<artdaq::Fragment::timestamp_t> timestamp_scale{fhicl::Name{"time_step_ms"}, fhicl::Comment{"Number of milliseconds between requests"}, 211};
    fhicl::Atom<int> run_number{fhicl::Name{"run_number"}, fhicl::Comment{"Run number"}, 0};
  };

  auto pset = LoadParameterSet<Config>(argc, argv, "sender", "This test application sends Data Request messages and optionally receives them to detect issues in the network transport");


  artdaq::RequestSender sender(pset);

  std::unique_ptr<artdaq::RequestReceiver> receiver(nullptr);
  int num_requests = pset.get<int>("num_requests", 1);
  if (pset.get<bool>("use_receiver", false)) {
    receiver.reset(new artdaq::RequestReceiver(pset.get<fhicl::ParameterSet>("receiver_config")));
    receiver->startRequestReception();
  }

  auto seq = pset.get<artdaq::Fragment::sequence_id_t>("starting_sequence_id", 1);
  auto seq_scale = pset.get<artdaq::Fragment::sequence_id_t>("sequence_id_step", 1);
  auto tmo = pset.get<size_t>("recevier_timeout_ms", 1000);
  int  run_number = pset.get<int>("run_number", 0);
  sender.SetRunNumber(run_number);

  std::chrono::time_point<std::chrono::system_clock> request_time = std::chrono::system_clock::now();
  std::chrono::duration<int, std::ratio<1, 1000>> time_step = std::chrono::milliseconds(
      pset.get<artdaq::Fragment::timestamp_t>("time_step_ms", 199)
      );
    
  TLOG(TLVL_INFO) << "Preparing to send " << num_requests << " requests separated by " << std::chrono::milliseconds(time_step).count()  << " ms";
  for (int ii = 0; ii < num_requests; ++ii) {
    request_time += time_step;
    std::this_thread::sleep_until (request_time);
    sender.AddRequest(seq, request_time.time_since_epoch().count());
    TLOG(TLVL_DEBUG) << "Sending request for Sequence ID " << seq << ", timestamp " << request_time.time_since_epoch().count();
    sender.SendRequest();

    if (receiver) {
      auto start_time = std::chrono::steady_clock::now();
      bool recvd = false;
      while (!recvd && artdaq::TimeUtils::GetElapsedTimeMilliseconds(start_time) < tmo) {
        auto reqs = receiver->GetRequests();
        if (reqs.count(seq)) {
          TLOG(TLVL_INFO) << "Received Request for Sequence ID " << seq << ", timestamp " << reqs[seq];
          receiver->RemoveRequest(seq);
          sender.RemoveRequest(seq);
          recvd = true;
        }
        else {
          usleep(100);
        }
      }
    } 

    seq += seq_scale;
  } 

  return 0;
}
