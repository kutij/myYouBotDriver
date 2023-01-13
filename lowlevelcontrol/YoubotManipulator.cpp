#include "YoubotManipulator.hpp"
#include <stdexcept>
#include "Logger.hpp"
#include "Time.hpp"

using namespace youbot;

YoubotManipulator::YoubotManipulator(const YoubotConfig& config, EtherCATMaster* center)
  : config(config), center(center) {
  // Check slavenames
  if (center->getSlaveNum() < 6)
	throw std::runtime_error("Less than 6 ethercat slaves");
  for (int i = 0; i < 5; i++) {
	if (config.jointIndices[i] >= center->getSlaveNum())
	  throw std::runtime_error("Slave index " + std::to_string(config.jointIndices[i]) + " not found");
	if (center->getSlaveName(config.jointIndices[i]).compare("TMCM-1610") != 0)
	  throw std::runtime_error("Unknown module name " + center->getSlaveName(config.jointIndices[i]));
	joints.push_back(std::make_shared<YoubotJoint>(config.jointIndices[i], config.jointConfigs[i], center));
  }

  // Set ProcessMsgFromSlave sizes
  for (int i = 0; i < 5; i++)
	center->SetProcessFromSlaveSize(20, config.jointIndices[i]);
}

YoubotJoint::Ptr YoubotManipulator::GetJoint(int i) {
  return joints[i];
}

void YoubotManipulator::ConfigJoints(bool forceConfiguration) {
  for (int i = 0; i < 5; i++)
	joints[i]->ConfigParameters(forceConfiguration);
}

bool YoubotManipulator::CheckJointConfigs() {
  for (int i = 0; i < 5; i++)
	if (!joints[i]->CheckConfig())
	  return false;
  return true;
}

void YoubotManipulator::InitializeAllJoints() {
  for (auto& it : joints) {
	it->Initialize();
	SLEEP_MILLISEC(500); // Wait to finish the moves - controller likes it
  }
}

enum CalibState : uint8_t {
  TO_CALIBRATE = 0,
  ENCODER_SETTING = 1,
  PEACE = 2,
  IDLE = 3
};

void YoubotManipulator::Calibrate(bool forceCalibration) {
  CalibState jointcalstate[5];
  const double calJointRadPerSec = 0.35;
  for (int i = 0; i < 5; i++) {
	// RESET I2t flags
	auto status = joints[i]->GetJointStatusViaMailbox();
	log(Log::info, status.toString());
	if (status.I2TExceeded())
	  joints[i]->ResetI2TExceededViaMailbox();
	// Check which joints needs calibration
	if (joints[i]->IsCalibratedViaMailbox() && !forceCalibration)
	  jointcalstate[i] = IDLE;
	else {
	  jointcalstate[i] = TO_CALIBRATE;
	  bool backward = bool(config.jointConfigs[i].at("CalibrationDirection"))
		^ bool(config.jointConfigs[i].at("qDirectionSameAsEnc"));
	  joints[i]->ReqJointSpeedRadPerSec(backward ? -calJointRadPerSec : calJointRadPerSec);
	  log(Log::info, "Calibration of joint " + std::to_string(i) + "started");
	}
  }
  // Reset timeouts (after all possible i2t reset or it can go to timeout again)
  for (int i = 0; i < 5; i++)
	if (jointcalstate[i] != IDLE)
	  joints[i]->ResetTimeoutViaMailbox();
  // Main calibration loop: move with constant speed and check if it has stopped
  auto start = std::chrono::steady_clock::now();
  center->ExchangeProcessMsg(); // do sg to avoid a new timout
  SLEEP_MILLISEC(2) // Wait until the status flag of process messages will be refreshed (without timeout flag)
  int cycles_in_zero_speed[5] = { 0,0,0,0,0 }; // counters for ~speed
  do {
	center->ExchangeProcessMsg();
	// Check status
	for (int i = 0; i < 5; i++)
	  if (jointcalstate[i] != IDLE) {
		auto status = joints[i]->GetProcessReturnData().status;
		if (status.I2TExceeded()) {
		  log(Log::fatal, "I2t exceeded during calibration in joint " + std::to_string(i) + " (" + status.toString() + ")");
		  SLEEP_MILLISEC(10);
		  throw std::runtime_error("I2t exceeded during calibration");
		}
		if (status.Timeout()) {
		  log(Log::fatal, "Timeout during calibration in joint " + std::to_string(i) + " (" + status.toString() + ")");
		  SLEEP_MILLISEC(10);
		  throw std::runtime_error("Timeout during calibration");
		}
	  }
	// Check if enough time elapsed (in the first 200ms, the joints can start to move)
	SLEEP_MILLISEC(3);
	auto end = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < 200)
	  continue;
	// Do calibration
	std::string str = "Calibration vel: ";
	for (int i = 0; i < 5; i++)
	  switch (jointcalstate[i]) {
	  case TO_CALIBRATE: {
		int vel = joints[i]->GetProcessReturnData().motorVelocityRPM;
		str = str + std::to_string(vel) + "RPM ";
		if (vel<5 && vel>-5)
		  cycles_in_zero_speed[i]++;
		else
		  cycles_in_zero_speed[i] = 0;
		if (cycles_in_zero_speed[i]>=5) {
		  log(Log::info, "Joint " + std::to_string(i) + " calibrated");
		  joints[i]->ReqEncoderReference(0);
		  jointcalstate[i] = ENCODER_SETTING;
		}
		break;
	  }
	  case ENCODER_SETTING: {
		int enc = joints[i]->GetProcessReturnData().encoderPosition;
		if (enc == 0) {
		  str = str + " to_set ";
		  joints[i]->ReqVoltagePWM(0);
		  jointcalstate[i] = PEACE;
		}
		else str = str + " under_set ";
	  }
	  case PEACE:
		str = str + " set ";
		break;
	  case IDLE:
		str = str + " - ";
		break;
	  }
	log(Log::info, str);
  } while (jointcalstate[0] < PEACE || jointcalstate[1] < PEACE ||
	jointcalstate[2] < PEACE || jointcalstate[3] < PEACE || jointcalstate[4] < PEACE);
  for (int i = 0; i < 5; i++)
	if (jointcalstate[i] == PEACE)
	  joints[i]->SetCalibratedViaMailbox();
  for (int i = 0; i < 5; i++)
	joints[i]->IsCalibratedViaMailbox();
  log(Log::info, "After calibration:");
  for (auto& it : joints)
	it->GetProcessReturnData().Print();
}

void YoubotManipulator::ReqJointPositionRad(double q0,
  double q1, double q2, double q3, double q4) {
  joints[0]->ReqJointPositionRad(q0);
  joints[1]->ReqJointPositionRad(q1);
  joints[2]->ReqJointPositionRad(q2);
  joints[3]->ReqJointPositionRad(q3);
  joints[4]->ReqJointPositionRad(q4);
}

void YoubotManipulator::GetJointPositionRad(double& q0,
  double& q1, double& q2, double& q3, double& q4) {
  q0 = joints[0]->GetJointPositionRad();
  q1 = joints[1]->GetJointPositionRad();
  q2 = joints[2]->GetJointPositionRad();
  q3 = joints[3]->GetJointPositionRad();
  q4 = joints[4]->GetJointPositionRad();
}

void YoubotManipulator::ReqManipulatorStop() {
  for (auto& it : joints)
	it->ReqStop();
}
void YoubotManipulator::ResetErrorFlags() {
  for (int i = 0; i < 5; i++) {
	auto status = joints[i]->GetJointStatusViaMailbox();
	if (status.I2TExceeded())
	  joints[i]->ResetI2TExceededViaMailbox();
  }
  for (int i = 0; i < 5; i++)
	joints[i]->ResetTimeoutViaMailbox();
}