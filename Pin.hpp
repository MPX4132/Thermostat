// =============================================================================
// Pin : This class abstracts the I/O pins found on the development board. The
// class keeps track of all pins available.
// =============================================================================

#include <vector>
#include <utility>

class Pin
{
public:
	typedef unsigned int Identifier;
	typedef int Value;

	enum Mode
	{
		Invalid,
		Auto,
		Output,
		Input
	};

	class Set
	{
	public:
		typedef std::pair<Identifier, Value> Member 
	
		bool ready() const;
		
		std::vector<Member> states() const;
		bool setStates(std::vector<Member> const states);
	
		Set(std::vector<Pin> const pins);
	};
	
	
	Identifier identity() const;
	
	bool ready() const;
	
	Mode mode() const;
	bool setMode(Mode const mode);
	
	
	Value state() const;
	bool setState(Value const state);
	
	Pin(Identifier const identifier);
	
protected:
	Identity const _identity;
	Mode _mode;
	

	static std::vector _reserved;
	
	static bool Reserve(Identifier const pinID);
	static bool Release(Identifier const pinID);
};