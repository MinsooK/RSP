
/**
 *  모듈의 파일 타입 반환 함수 구현
 *   @file Module.cpp
 *   @date 2017-04-20
 *   @author Park donghyeon
 *   @brief 모듈 클래스 정의
 */
#include <Module.hpp>

#include <stdexcept>

#include <bicomc/detail/atomic.h>

namespace {
	void onInitilizeDummy(irp::Property const& property) {}
	void onStartDummy() {}
	void onExecuteDummy() {}
	void onStopDummy() {}
	void onDestroyDummy() {}
	bool onErrorDummy(bcc::int32_t prevAction) { return false; }
}

Module::Module()
	: mState(ModuleState::CREATED), _name(), _moduleType(ModuleType::THREAD)
	, _operationType(ScheduleType::PERIODIC), _period(), _priority(), mCallback()
{
	clearCallbacks();
}

Module::Module(std::string name, ModuleType type, ScheduleType schedule
	, bcc::int64_t period, int prior)
	: mState(ModuleState::CREATED), _moduleType(type)
	, _operationType(schedule), _period(period), _priority(prior)
	, mCallback()
{
	_name.swap(name);
	clearCallbacks();
}

Module::~Module()
{}

void Module::initialize()
{
	if (!(mState == ModuleState::CREATED || mState == ModuleState::ERROR))
		throw std::logic_error("Module::initialize() : '" + std::string(mState.c_str()) + "' is invalid precondition.");

	try
	{
		mCallback.onInitialize(mProperty);
		mState = ModuleState::PAUSED;
	}
	catch (...)
	{
		error(ModuleAction::INITIALIZE);
	}
}

void Module::start()
{
	if (!(mState == ModuleState::PAUSED || mState == ModuleState::ERROR))
		throw std::logic_error("Module::start() : '" + std::string(mState.c_str()) + "' is invalid precondition.");

	try
	{
		mCallback.onStart();
		mState = ModuleState::EXECUTING;
	}
	catch (...)
	{
		error(ModuleAction::START);
	}
}

void Module::execute(bcc::int64_t ns)
{
	if (!(mState == ModuleState::EXECUTING || mState == ModuleState::ERROR))
		throw std::logic_error("Module::execute() : '" + std::string(mState.c_str()) + "' is invalid precondition.");

	try
	{
		mLastExecutedTime = ns;
		mCallback.onExecute();
		mState = ModuleState::EXECUTING;
	}
	catch (...)
	{
		error(ModuleAction::EXECUTE);
	}
}

void Module::stop()
{
	if (!(mState == ModuleState::EXECUTING || mState == ModuleState::ERROR))
		throw std::logic_error("Module::stop() : '" + std::string(mState.c_str()) + "' is invalid precondition.");

	try
	{
		mCallback.onStop();
		mState = ModuleState::PAUSED;
	}
	catch (...)
	{
		error(ModuleAction::STOP);
	}
}

void Module::destroy()
{
	if (mState == ModuleState::DESTRUCTED)
		return;

	if (mState == ModuleState::EXECUTING)
		stop();

	if (!(mState == ModuleState::PAUSED || mState == ModuleState::ERROR))
		throw std::logic_error("Module::destroy() : '" + std::string(mState.c_str()) + "' is invalid precondition.");

	try
	{
		mCallback.onDestroy();
	}
	catch (...)
	{}

	mState = ModuleState::DESTRUCTED;
}

void Module::setOnInitialize(OnInitializeCallback callback)
{
	if (callback) mCallback.onInitialize = callback;
	else mCallback.onInitialize = &onInitilizeDummy;
}

void Module::setOnStart(OnStartCallback callback)
{
	if (callback) mCallback.onStart = callback;
	else mCallback.onStart = &onStartDummy;
}

void Module::setOnExecute(OnExecuteCallback callback)
{
	if (callback) mCallback.onExecute = callback;
	else mCallback.onExecute = &onExecuteDummy;
}

void Module::setOnStop(OnStopCallback callback)
{
	if (callback) mCallback.onStop = callback;
	else mCallback.onStop = &onStopDummy;
}

void Module::setOnDestroy(OnDestroyCallback callback)
{
	if (callback) mCallback.onDestroy = callback;
	else mCallback.onDestroy = &onDestroyDummy;
}

void Module::setOnError(OnErrorCallback callback)
{
	if (callback) mCallback.onError = callback;
	else mCallback.onError = &onErrorDummy;
}

void Module::clearCallbacks()
{
	setOnInitialize(&onInitilizeDummy);
	setOnStart(&onStartDummy);
	setOnExecute(&onExecuteDummy);
	setOnStop(&onStopDummy);
	setOnDestroy(&onDestroyDummy);
	setOnError(&onErrorDummy);
}

void Module::error(ModuleAction prevAction)
{
	if (mState == ModuleState::DESTRUCTED)
		return;

	mState = ModuleState::ERROR;

	try
	{
		if (mCallback.onError(prevAction))
		{
			recovery(prevAction);
			return;
		}
	}
	catch (...)
	{}

	destroy();
}

void Module::recovery(ModuleAction prevAction)
{
	if (mState != ModuleState::ERROR)
		throw std::logic_error("Module::recovery() : '" + std::string(mState.c_str()) + "' is invalid precondition.");

	switch (prevAction)
	{
	case ModuleAction::INITIALIZE:
		initialize();
		return;
	case ModuleAction::START:
		start();
		return;

	case ModuleAction::EXECUTE:
		execute(mLastExecutedTime);
		return;

	case ModuleAction::STOP:
		stop();
		return;

	default:
		throw std::invalid_argument("Module::recovery() : '" + std::string(prevAction.c_str()) + "' action cannot be recoveried.");
	}
}
