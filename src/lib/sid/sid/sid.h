/******************************************************************************/
/*
  Author  - Ming-Lun "Allen" Chou
  Web     - http://AllenChou.net
  Twitter - @TheAllenChou
*/
/******************************************************************************/

#pragma once


// StringId declaration
//-----------------------------------------------------------------------------

constexpr const unsigned long long StringIdHashConcat(unsigned long long base, const char *str)
{
  return (*str) ? (StringIdHashConcat((base ^ *str) * 0x100000001b3 , str + 1)) : base;
}

constexpr const unsigned long long StringIdHash(const char *str)
{
  return StringIdHashConcat(0xcbf29ce484222325, str);
}

class StringId
{
  public:
    
    typedef unsigned long long Storage;
    static_assert(sizeof(Storage) == 8, "StringId requires 64-bit storage");

    StringId() : StringId(static_cast<Storage>(0)) { }
    explicit StringId(Storage data) : m_data(data) { }
    explicit StringId::StringId(const char *str) : m_data(StringIdHash(str)) { }

    static const StringId StringId::Concat(const StringId &sid, const char *str)
    {
      return sid.Concat(str);
    }

    const StringId StringId::Concat(const char *str) const
    {
      return StringId(StringIdHashConcat(m_data, str));
    }

    const Storage GetValue() const { return m_data; }

  private:
    
    Storage m_data;
};

static std::ostream& operator<<(std::ostream &out, StringId sid)
{
    return out << "sid:" << sid.GetValue();
}

static const bool operator==(const StringId& lhs, const StringId &rhs)
{
    return lhs.GetValue() == rhs.GetValue();
}

static const bool operator!=(const StringId& lhs, const StringId &rhs)
{
  return lhs.GetValue() != rhs.GetValue();
}

//-----------------------------------------------------------------------------
// end: stringId declaration


// FNV-1a hash
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//-----------------------------------------------------------------------------

// disable overflow warnings due to intentional large integer multiplication
#pragma warning (disable: 4307)

//-----------------------------------------------------------------------------
// end FNV-1a hash


// StringId macros & constants
//-----------------------------------------------------------------------------

#define SID(str) (StringId(str))
#define SID_VAL(str) (StringIdHash(str))
static const StringId kInvalidStringId;

//-----------------------------------------------------------------------------
// end: StringId macros & constant
