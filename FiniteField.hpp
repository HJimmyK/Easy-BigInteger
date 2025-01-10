/*
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky

https://github.com/Twilight-Dream-Of-Magic/
https://github.com/With-Sky

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef FINITE_FIELD_HPP
#define FINITE_FIELD_HPP

#include "BigInteger.hpp"
#include "PrimeNumberTester.hpp"
#include <stdexcept>
#include <string>
#include <memory>

namespace TwilightDream::Math
{
	// ���࣬�������й�����߼�
	class FiniteFieldBase
	{
	public:
		using Integer = TwilightDream::BigInteger::BigSignedInteger;
		using UnsignedInteger = TwilightDream::BigInteger::BigInteger;
		using Montgomery = TwilightDream::BigInteger::Montgomery;

	protected:
		UnsignedInteger prime;

		// ���캯��
		FiniteFieldBase( const UnsignedInteger& modulus ) : prime( modulus )
		{}

	public:
		// ����������
		virtual ~FiniteFieldBase() = default;

		// ������Ԫ�������߼�
		virtual Integer compute_inverse_signed() const
		{
			std::cerr << "����: ���û���� compute_inverse_signed ������" << std::endl;
			throw std::runtime_error( "Called base class compute_inverse_signed method." );
		}

		virtual UnsignedInteger compute_inverse_unsigned() const
		{
			std::cerr << "����: ���û���� compute_inverse_unsigned ������" << std::endl;
			throw std::runtime_error( "Called base class compute_inverse_unsigned method." );
		}

		// ��ȡģ��
		virtual const UnsignedInteger& GetPrime() const
		{
			return prime;
		}
	};

	class UnsignedFiniteField;
	class SignedFiniteField;

	class UnsignedFiniteField : public FiniteFieldBase
	{
	protected:
		UnsignedInteger unsigned_value;
		bool			is_tested_prime = false;

		UnsignedInteger compute_inverse_unsigned() const
		{
			return this->power( prime - UnsignedInteger( "2" ) ).unsigned_value;
		}
	public:
		// ���캯��
		UnsignedFiniteField( const UnsignedInteger& number, const UnsignedInteger& modulus )
			:
			unsigned_value( number ),
			FiniteFieldBase( modulus )
		{
			if(!is_tested_prime)
			{
				this->SetPrime(modulus);
			}
			unsigned_value = number % prime;
		}

		UnsignedFiniteField( const std::string& number, const std::string& modulus )
			:
			UnsignedFiniteField( UnsignedInteger(number, 10), UnsignedInteger( modulus, 10 ) )
		{
			unsigned_value = unsigned_value % prime;
		}

		UnsignedFiniteField( const std::string& number, uint32_t base_number, const std::string& modulus, uint32_t base_modulus = 10 )
			:
			UnsignedFiniteField( UnsignedInteger(number, base_number) , UnsignedInteger( modulus, base_modulus ) )
		{
			unsigned_value = unsigned_value % prime;
		}

		// ���ƹ��캯��
		UnsignedFiniteField( const UnsignedFiniteField& other )
			:
			unsigned_value( other.unsigned_value ),
			is_tested_prime( other.is_tested_prime ),
			FiniteFieldBase( other.prime )
		{}

		// �ƶ����캯��
		UnsignedFiniteField( UnsignedFiniteField&& other ) noexcept
			:
			unsigned_value( std::move(other.unsigned_value) ),
			is_tested_prime( std::move( other.is_tested_prime ) ),
			FiniteFieldBase( std::move( other.prime ) )
		{}

		// ��ֵ�����
		UnsignedFiniteField& operator=( const UnsignedFiniteField& other )
		{
			if ( this != &other )
			{
				if ( prime != other.prime )
				{
					throw std::invalid_argument( "Cannot assign elements from different fields." );
				}
				this->unsigned_value = other.unsigned_value;
				this->is_tested_prime = other.is_tested_prime;
			}
			return *this;
		}

		// ��ֵ��������ƶ���
		UnsignedFiniteField& operator=( UnsignedFiniteField&& other )
		{
			if ( this != &other )
			{
				this->unsigned_value = std::move( other.unsigned_value );
				this->prime = std::move( other.prime );
				this->is_tested_prime = other.is_tested_prime;
			}
			return *this;
		}

		// ��ȡ�޷���ֵ
		const UnsignedInteger& GetValue() const
		{
			return unsigned_value;
		}

		void SetValue( const UnsignedFiniteField& value ) 
		{
			unsigned_value = value.unsigned_value;
		}

		// �ӷ�
		UnsignedFiniteField operator+( const UnsignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot add elements from different fields." );
			}
			return UnsignedFiniteField( unsigned_value + other.unsigned_value, prime );
		}

		// ����
		UnsignedFiniteField operator-( const UnsignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot subtract elements from different fields." );
			}
			
			if(this->unsigned_value > other.unsigned_value)
			{
				return UnsignedFiniteField( ( unsigned_value - other.unsigned_value ) % prime, prime );
			}
			else
			{
				UnsignedInteger difference = other.unsigned_value - unsigned_value;
				UnsignedInteger diveded_difference = difference / prime;
				UnsignedInteger result = diveded_difference * prime - difference;
				return UnsignedFiniteField( result, prime );
			}
		}

		// �˷�
		UnsignedFiniteField operator*( const UnsignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot multiply elements from different fields." );
			}
			UnsignedInteger result;
			if ( unsigned_value.BitLength() > 128 || other.unsigned_value.BitLength() > 128 )
			{
				//�ɸ������㷨
				Montgomery montgomery( this->prime );
				result = montgomery.Multiplication(unsigned_value, other.unsigned_value);
				return UnsignedFiniteField( result, prime );
			}
			return UnsignedFiniteField( unsigned_value * other.unsigned_value, prime );
		}

		// ����
		UnsignedFiniteField operator/( const UnsignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot divide elements from different fields." );
			}
			UnsignedInteger inverse = other.compute_inverse_unsigned();
			return UnsignedFiniteField( unsigned_value * inverse, prime );
		}

		// ���Ƹ�ֵ
		UnsignedFiniteField& operator<<=( size_t shift )
		{
			if ( shift == 0 )
				return *this;

			// ��λ����
			unsigned_value <<= shift;

			if(unsigned_value >= prime)
				unsigned_value %= prime;

			return *this;
		}

		// ���Ƹ�ֵ
		UnsignedFiniteField& operator>>=( size_t shift )
		{
			if ( shift == 0 )
				return *this;

			// ��λ����
			unsigned_value >>= shift;

			if(unsigned_value >= prime)
				unsigned_value %= prime;

			return *this;
		}

		// ����
		UnsignedFiniteField operator<<( const size_t& shift ) const
		{
			UnsignedFiniteField copy = *this;
			copy <<= shift;
			return copy;
		}

		// ����
		UnsignedFiniteField operator>>( const size_t& shift ) const
		{
			UnsignedFiniteField copy = *this;
			copy >>= shift;
			return copy;
		}

		// ������
		UnsignedFiniteField power( const UnsignedInteger& exponent ) const
		{
			UnsignedInteger result = UnsignedInteger( "1" );
			UnsignedInteger base = unsigned_value;
			UnsignedInteger copy_exponent = exponent;

			if ( copy_exponent.IsZero() )
			{
				return UnsignedFiniteField( result, prime );
			}

			if ( copy_exponent.IsNegative() )
			{
				throw std::invalid_argument( "Negative exponent not supported for PositiveFiniteField." );
			}

			if ( base.BitLength() > 128 )
			{
				Montgomery montgomery( this->prime );
				result = montgomery.Power(base, copy_exponent);
				return UnsignedFiniteField( result, prime );
			}
			else
			{
				while ( !copy_exponent.IsZero() )
				{
					if (copy_exponent & 1)
					{
						result = ( result * base );
					}
					base = ( base * base );
					copy_exponent >>= 1;
				}
			}

			return UnsignedFiniteField( result, prime );
		}

		// ����Ԫ
		UnsignedFiniteField inverse() const
		{
			if(unsigned_value == 1)
			{
				return UnsignedFiniteField( unsigned_value, prime );
			}

			UnsignedInteger result = compute_inverse_unsigned();
			return UnsignedFiniteField( result, prime );
		}

		// ȡ�� ��ת���� �з���������
		SignedFiniteField Convert() const;

		// ����Լ��
		bool operator==( const UnsignedFiniteField& other ) const
		{
			return ( unsigned_value == other.unsigned_value ) && ( prime == other.prime );
		}

		bool operator!=( const UnsignedFiniteField& other ) const
		{
			return !( *this == other );
		}

		// ת��Ϊ�ַ���
		std::string ToString() const
		{
			return "PositiveFiniteField_" + prime.ToString() + "(" + unsigned_value.ToString() + ")";
		}

		// ����ģ���������Ҫ��
		void SetPrime( const UnsignedInteger& modulus )
		{
			using PrimeNumberTester = TwilightDream::PrimeNumberTester;

			if ( !is_tested_prime )
			{
				PrimeNumberTester PrimeTester;
				if ( !PrimeTester.IsPrime( modulus ) )
				{
					is_tested_prime = false;
					throw std::invalid_argument( "The modulus must be a prime number." );
				}
				is_tested_prime = true;
			}

			this->prime = modulus;
			this->unsigned_value = this->unsigned_value % prime;
		}
	};

	class SignedFiniteField : public FiniteFieldBase
	{
	protected:
		Integer			signed_value;
		bool			is_tested_prime = false;

		Integer compute_inverse_signed() const
		{
			// ��չŷ������㷨
			Integer t = 0;
			Integer new_t = 1;
			Integer r = prime;
			Integer new_r = signed_value;

			Integer q;
			Integer temp_t;
			Integer temp_r;
			while ( !new_r.IsZero() )
			{
				q = r / new_r;
				temp_t = t - q * new_t;
				t = new_t;
				new_t = temp_t;

				temp_r = r - q * new_r;
				r = new_r;
				new_r = temp_r;
			}

			if ( r > Integer( "1" ) )
			{ 
				throw std::invalid_argument( "Element has no inverse in the field." );
			}

			return t;
		}
	public:
		// ���캯��
		SignedFiniteField( const Integer& number, const UnsignedInteger& modulus )
			:
			signed_value( number ),
			FiniteFieldBase( modulus )
		{
			if(!is_tested_prime)
			{
				this->SetPrime(modulus);
			}
			signed_value = number % prime;
		}

		SignedFiniteField( const std::string& number, const std::string& modulus )
			:
			SignedFiniteField( Integer(number, 10) , UnsignedInteger( modulus, 10 ) )
		{
			signed_value = signed_value % prime;
		}

		SignedFiniteField( const std::string& number, uint32_t base_number, const std::string& modulus, uint32_t base_modulus = 10 )
			:
			SignedFiniteField( Integer(number, base_number) , UnsignedInteger( modulus, base_modulus ) )
		{
			signed_value = signed_value % prime;
		}

		// ���ƹ��캯��
		SignedFiniteField( const SignedFiniteField& other )
			:
			signed_value( other.signed_value ),
			is_tested_prime( other.is_tested_prime ),
			FiniteFieldBase( other.prime )
		{}

		// �ƶ����캯��
		SignedFiniteField( SignedFiniteField&& other ) noexcept
			:
			signed_value( std::move(other.signed_value) ),
			is_tested_prime( std::move( other.is_tested_prime ) ),
			FiniteFieldBase( std::move( other.prime ) )
		{}

		// ��ֵ�����
		SignedFiniteField& operator=( const SignedFiniteField& other )
		{
			if ( this != &other )
			{
				if ( prime != other.prime )
				{
					throw std::invalid_argument( "Cannot assign elements from different fields." );
				}
				this->signed_value = other.signed_value;
				this->is_tested_prime = other.is_tested_prime;
			}
			return *this;
		}

		// ��ֵ��������ƶ���
		SignedFiniteField& operator=( SignedFiniteField&& other )
		{
			if ( this != &other )
			{
				this->signed_value = std::move( other.signed_value );
				this->prime = std::move( other.prime );
				this->is_tested_prime = other.is_tested_prime;
			}
			return *this;
		}

		// ��ȡ�з���ֵ
		const Integer& GetValue() const
		{
			return signed_value;
		}

		void SetValue( const SignedFiniteField& value ) 
		{
			signed_value = value.signed_value;
		}

		// ���������

		// �ӷ�
		SignedFiniteField operator+( const SignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot add elements from different fields." );
			}
			return SignedFiniteField( signed_value + other.signed_value, prime );
		}

		// ����
		SignedFiniteField operator-( const SignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot subtract elements from different fields." );
			}
			return SignedFiniteField( signed_value - other.signed_value, prime );
		}

		// �˷�
		SignedFiniteField operator*( const SignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot multiply elements from different fields." );
			}
			return SignedFiniteField( signed_value * other.signed_value, prime );
		}

		// ����
		SignedFiniteField operator/( const SignedFiniteField& other ) const
		{
			if ( prime != other.prime )
			{
				throw std::invalid_argument( "Cannot divide elements from different fields." );
			}
			Integer inverse = other.compute_inverse_signed();
			return SignedFiniteField( signed_value * inverse, prime );
		}

		// ���Ƹ�ֵ
		SignedFiniteField& operator<<=( size_t shift )
		{
			if ( shift == 0 )
				return *this;

			// ��λ����
			signed_value <<= shift;

			if(signed_value >= prime)
				signed_value %= prime;

			return *this;
		}

		// ���Ƹ�ֵ
		SignedFiniteField& operator>>=( size_t shift )
		{
			if ( shift == 0 )
				return *this;

			// ��λ����
			signed_value >>= shift;

			if(signed_value >= prime)
				signed_value %= prime;

			return *this;
		}

		// ����
		SignedFiniteField operator<<( const size_t& shift ) const
		{
			SignedFiniteField copy = *this;
			copy <<= shift;
			return copy;
		}

		// ����
		SignedFiniteField operator>>( const size_t& shift ) const
		{
			SignedFiniteField copy = *this;
			copy >>= shift;
			return copy;
		}

		// ������
		SignedFiniteField power( const Integer& exponent ) const
		{
			Integer result = Integer( "1" );
			Integer base = signed_value;
			Integer copy_exponent = exponent;

			// ����ָ��
			if ( exponent.IsNegative() )
			{
				copy_exponent = -exponent;
				base = compute_inverse_signed();
			}

			while ( !copy_exponent.IsZero() )
			{
				if ( ( copy_exponent % Integer( "2" ) ) == Integer( "1" ) )
				{
					result = ( result * base );
				}
				base = ( base * base );
				copy_exponent >>= 1;
			}

			return SignedFiniteField( result, prime );
		}

		// ����Ԫ
		SignedFiniteField inverse() const
		{
			if(signed_value == 1)
			{
				return SignedFiniteField( signed_value, prime );
			}

			Integer result = compute_inverse_signed();
			return SignedFiniteField( result, prime );
		}

		// ȡ�� ��ת���� �޷���������
		UnsignedFiniteField Convert() const;

		// ����Լ��
		bool operator==( const SignedFiniteField& other ) const
		{
			return ( signed_value == other.signed_value ) && ( prime == other.prime );
		}

		bool operator!=( const SignedFiniteField& other ) const
		{
			return !( *this == other );
		}

		// ת��Ϊ�ַ���
		std::string ToString() const
		{
			return "NegativeFiniteField_" + prime.ToString() + "(" + signed_value.ToString() + ")";
		}

		// ����ģ���������Ҫ��
		void SetPrime( const Integer& modulus )
		{
			using PrimeNumberTester = TwilightDream::PrimeNumberTester;

			if ( modulus.IsNegative() )
			{
				throw std::invalid_argument( "The modulus must be a positive integer." );
			}

			if ( !is_tested_prime )
			{
				PrimeNumberTester PrimeTester;
				if ( !PrimeTester.IsPrime( modulus.Abs() ) )
				{
					is_tested_prime = false;
					throw std::invalid_argument( "The modulus must be a prime number." );
				}
				is_tested_prime = true;
			}

			this->prime = modulus;
			this->signed_value = this->signed_value % prime;
		}
	};

	inline SignedFiniteField UnsignedFiniteField::Convert() const
	{
		if ( unsigned_value.IsZero() )
		{
			return SignedFiniteField( unsigned_value, prime );
		}
		
		FiniteFieldBase::UnsignedInteger prime = this->GetPrime();
		FiniteFieldBase::Integer result = (-FiniteFieldBase::Integer(unsigned_value)) % prime;
		return SignedFiniteField( result, prime );
	}

	inline UnsignedFiniteField SignedFiniteField::Convert() const
	{
		if ( signed_value.IsZero() )
		{
			return UnsignedFiniteField( signed_value, prime );
		}
			
		FiniteFieldBase::UnsignedInteger prime = this->GetPrime();
		FiniteFieldBase::UnsignedInteger result = FiniteFieldBase::UnsignedInteger(-signed_value) % prime;
		return UnsignedFiniteField( result, prime );
	}

}  // namespace TwilightDream::Math

#endif	// FINITE_FIELD_HPP
