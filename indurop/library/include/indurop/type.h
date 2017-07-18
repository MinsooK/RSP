
#ifndef INDUROP_TYPE_H__
#define INDUROP_TYPE_H__

namespace irp {

	struct ModuleType
	{
		enum type
		{
			THREAD = 0 // 공유라이브러리 모듈
			, PROCESS = 1 // 실행파일 모듈
		};

		ModuleType(type value_) : mValue(value_) {}
		operator type() const { return mValue; }
		type value() const { return mValue; }

		char const* c_str() const
		{
			switch (mValue)
			{
			case (ModuleType::THREAD) : return "THREAD";
			case (ModuleType::PROCESS) : return "PROCESS";
			}
			return "invalid value";
		}

		type mValue;
	};

	struct ScheduleType
	{
		enum type
		{
			PERIODIC = 1, SPORADIC = 2, NON_REAL = 3
		};

		ScheduleType(type value_) : mValue(value_) {}
		operator type() const { return mValue; }
		type value() const { return mValue; }

		char const* c_str() const
		{
			switch (mValue)
			{
			case (ScheduleType::PERIODIC) : return "PERIODIC";
			case (ScheduleType::SPORADIC) : return "SPORADIC";
			case (ScheduleType::NON_REAL) : return "NON_REAL";
			}
			return "invalid value";
		}

		type mValue;
	};

	struct ModuleState
	{
		enum type
		{
			CREATED, PAUSED, EXECUTING, DESTRUCTED, ERROR
		};

		ModuleState(type value_) : mValue(value_) {}
		operator type() const { return mValue; }
		type value() const { return mValue; }

		char const* c_str() const
		{
			switch (mValue)
			{
			case (ModuleState::CREATED) : return "CREATED";
			case (ModuleState::PAUSED) : return "PAUSED";
			case (ModuleState::EXECUTING) : return "EXECUTING";
			case (ModuleState::DESTRUCTED) : return "DESTRUCTED";
			case (ModuleState::ERROR) : return "ERROR";
			}
			return "invalid value";
		}

		type mValue;
	};

	struct ModuleAction
	{
		enum type
		{
			INITIALIZE, START, EXECUTE, STOP
			, DESTROY, ERROR, RECOVERY
		};

		ModuleAction(type value_) : mValue(value_) {}
		operator type() const { return mValue; }
		type value() const { return mValue; }

		char const* c_str() const
		{
			switch (mValue)
			{
			case (ModuleAction::INITIALIZE) : return "INITIALIZE";
			case (ModuleAction::START) : return "START";
			case (ModuleAction::EXECUTE) : return "EXECUTE";
			case (ModuleAction::STOP) : return "STOP";
			case (ModuleAction::DESTROY) : return "DESTROY";
			case (ModuleAction::ERROR) : return "ERROR";
			case (ModuleAction::RECOVERY) : return "RECOVERY";
			}
			return "invalid value";
		}

		type mValue;
	};

} // namespace irp

#endif // !def INDUROP_TYPE_H__
