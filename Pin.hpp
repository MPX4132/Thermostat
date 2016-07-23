// =============================================================================
// Pin : This class abstracts the I/O pins found on the development board. The
// class keeps track of all pins available.
// =============================================================================

class Pin
{
public:
	typedef unsigned long Identifier;

	class Set
	{
	};
	
	
	bool ready() const;
	
	Pin(Identifier const identifier);
};