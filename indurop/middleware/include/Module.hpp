
/*****************************************************
*   @file Module.h                                    *
*   @date 2017-04-20                                  *
*   @author Park Donghyeon                            *
*   @brief 모듈 클래스 선언                            *
*   @todo #define -> enum class 타입으로 변경해야 함.   *
*******************************************************/

#include <string>

#include <bicomc/stdint.h>

#include <indurop/property.h>
#include <indurop/type.h>
#include <indurop/detail/shared_ptr.h>

/******************************
 *   모듈 클래스                 * *
 *   @author Park Donghyeon * *
 *   @date   2017-05-01     * *
 ******************************/

class Module
{
public:
	typedef irp::ModuleType ModuleType;
	typedef irp::ScheduleType ScheduleType;
	typedef irp::ModuleState ModuleState;
	typedef irp::ModuleAction ModuleAction;

	typedef void(*OnInitializeCallback)(irp::Property const& property);
	typedef void(*OnStartCallback)();
	typedef void(*OnExecuteCallback)();
	typedef void(*OnStopCallback)();
	typedef void(*OnDestroyCallback)();
	typedef bool(*OnErrorCallback)(bcc::int32_t prevAction);

	static_assert(sizeof(bool) == sizeof(char), "'OnErrorCallback' may be incompatible.");

public:
	Module();

	Module(std::string name, ModuleType type, ScheduleType schedule, bcc::int64_t period, int prior); //< module -> type 으로 변경 예정

	~Module();

public:
	std::string const& getName() const { return _name; }
	// ModuleType getModuleType() { return _moduleType;}
	// ModeType getOperationType() { return _operationType;}
	ModuleType getModuleType() const { return _moduleType; }
	ScheduleType getOperationType() const { return _operationType; }
	ScheduleType getScheduleType() const { return _operationType; }
	bcc::int64_t getPeriod() const { return _period; }
	int getPriority() const { return _priority; }

public:
	void initialize();
	void start();
	void execute(bcc::int64_t ns); // 매　주기마다　호출
	void stop();
	void destroy();

	ModuleState state() const { return mState; }
	irp::Property& property() { return mProperty; }
	irp::Property const& property() const { return mProperty; }

public:
	void setOnInitialize(OnInitializeCallback callback);
	void setOnStart(OnStartCallback callback);
	void setOnExecute(OnExecuteCallback callback);
	void setOnStop(OnStopCallback callback);
	void setOnDestroy(OnDestroyCallback callback);
	void setOnError(OnErrorCallback callback);

	void clearCallbacks();

public:
	template<typename Deleter>
	void setOwner(void* handle, Deleter deleter);

private:
	void error(ModuleAction prevAction);
	void recovery(ModuleAction prevAction);

private:
	ModuleState mState;
	bcc::int64_t mLastExecutedTime;

	std::string _name;  ///< 경로를 포함한 파일명
	// ModuleType _moduleType;
	// ModeType _operationType;
	ModuleType _moduleType;
	ScheduleType _operationType;
	bcc::int64_t _period;
	int _priority;

	struct
	{
		OnInitializeCallback onInitialize;
		OnStartCallback onStart;
		OnExecuteCallback onExecute;
		OnStopCallback onStop;
		OnDestroyCallback onDestroy;
		OnErrorCallback onError;
	} mCallback;
	irp::SharedPtr<char> mpOwner;

	irp::detail::Property mProperty;
};

namespace detail {
	template<typename Deleter>
	struct ModuleOwnerDeleter
	{
		ModuleOwnerDeleter(Deleter deleter_) : deleter(deleter_) {}
		void operator()(char* p) const { deleter(static_cast<void*>(p)); }
		Deleter deleter;
	};
} // namespace detail

template<typename Deleter>
void Module::setOwner(void* handle, Deleter deleter)
{
	mpOwner.reset(static_cast<char*>(handle), detail::ModuleOwnerDeleter<Deleter>(deleter));
}
