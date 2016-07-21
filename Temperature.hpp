// =============================================================================
// Temperature : This class abstracts different types of temperatures and sets
// the international standard as the base temperature.
// =============================================================================

template<typename NumericType>
class Temperature 
{
public:
	enum Scale 
	{
		Internal,
		Celsius,
		Fahrenheit
	};
	
	bool operator==(const Temperature<NumericType>& other) const
	{
		return this->value() == other->value(this->scale());
	}
	
	bool operator<(const Temperature<NumericType>& other) const
	{
		return this->value() < other->value(this->scale());
	}
	
	bool operator>(const Temperature<NumericType>& other) const
	{
		return this->value() > other->value(this->scale());
	}
	
	Temperature& operator=(const Temperature<NumericType>& newValue)
	{
		this->setValue(newValue);
		return *this;
	}
	
	Temperature& operator=(const NumericType newValue)
	{
		this->setValue(newValue);
		return *this;
	}
	
	Scale scale() const
	{
		return this->_scale;
	}
	
	void setScale(const Scale newScale, bool updateValue = true)
	{
		// If it's the "Internal" scale, fuck off...
		if (newScale == Internal) return;
	
		if (updateValue)
			this->setValue(Convert(this->value(), this->scale(), newScale));
		
		_scale = newScale;
	}
	
	NumericType value(const Scale scale = Internal) const
	{
		return Convert(_value, Celsius, scale == Internal? this->scale():scale);
	}
	
	void setValue(const NumericType value)
	{
		_value = value;
	}
	
	void setValue(const Temperature<NumericType>& temperature)
	{
		this->setScale(temperature.scale(), false);
		this->setValue(temperature.value());
	}
	
	static NumericType Convert(const Temperature<NumericType>& temperature,
								const Scale outputScale)
	{
		return Convert(temperature.value(), temperature.scale(), outputScale);
	}
	
	static NumericType Convert(const NumericType value, 
								const Scale inputScale,
								const Scale outputScale)
	{
		// If we're not really converting, skip everything...
		if (inputScale == outputScale) return value;
		//if (inputScale == Internal || outputScale == Internal) return value;
		
		// Fugly code below, brace yourself... Can't think of anything better...
		switch (inputScale) 
		{
			case Celsius: 
			{
				switch (outputScale) 
				{
					case Fahrenheit:
						return (value + 40) * 1.80 - 40;
				}
			}	break;
			
			case Fahrenheit: 
			{
				switch (outputScale) 
				{
					case Celsius:
						return (value + 40) / 1.80 - 40;
				}
			}	break;
			
			default:
				break;
		} 
		
		// If everything failed, idk...
		return value;
	}
								
	Temperature(const Temperature<NumericType>& temperature)
	{
		this->setValue(temperature);
	}
	
	Temperature(const NumericType value = 72, const Scale scale = Fahrenheit):
	_value(value),
	_scale(scale)
	{
		if (this->scale() == Internal) 
			this->setScale(Fahrenheit, false);
	}

protected:
	NumericType _value;
	Scale _scale;
};