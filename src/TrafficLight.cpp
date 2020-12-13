#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

// Generic message queue for typename T.  
// Wait for and receive new messages and pull them from the queue. 
// Return the received object.
template <typename T>
T MessageQueue<T>::receive()
{
  std::unique_lock<std::mutex> lck(_mutex);
  _condition.wait(lck, [this] { return !_queue.empty(); });
  T msg = std::move(_queue.back());
  // _queue.pop_back();
  _queue.clear();
  return msg;
}

// Generic message queue for typename T.
// Add a new message to the queue and send a notification back to the receiving end.
template <typename T>
void MessageQueue<T>::send(T&& msg)
{
  std::lock_guard<std::mutex> lck(_mutex);
  std::cout << "Message " << msg << " has been sent to the queue." << std::endl;
  _queue.push_back(std::move(msg));
  _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

// Continuously runs and calls the receive function on the message queue. 
// Once it receives TrafficLightPhase::green, the method returns.
void TrafficLight::waitForGreen()
{
  while (true) {
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    TrafficLightPhase phase = messageQueue.receive();
    if (phase == TrafficLightPhase::green) {
      return;
    } 
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

// Start cycleThroughPhases() in a thread when the public method simulate() is called.
void TrafficLight::simulate()
{
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
// An infinite loop that measures the time between two loop cycles and toggles the current phase of the traffic light between red and green.
// Send an update method to the message queue when traffic light turns.
// Each cycle duration is a random value between 4 and 6 seconds. 
void TrafficLight::cycleThroughPhases()
{

  auto timeOfLastCycle = std::chrono::system_clock::now();
  auto timeOfCurrentCycle = std::chrono::system_clock::now();
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<int> distribution(4000, 6000);
  int cycleTime =  distribution(eng);
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timeOfCurrentCycle = std::chrono::system_clock::now();
    long time = std::chrono::duration_cast<std::chrono::milliseconds>(timeOfCurrentCycle - timeOfLastCycle).count();
    // std::cout << time << std::endl;
    if (time >= cycleTime) {
      
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
        timeOfLastCycle = std::chrono::system_clock::now();
      } else {
        _currentPhase = TrafficLightPhase::red;
        timeOfLastCycle = std::chrono::system_clock::now();
      }
      messageQueue.send(std::move(_currentPhase));
    }
  }
}

