#include "sf/config/Autocode.hpp"

Autocode::Autocode(std::ostream& kOs) : mOs(kOs), mIndentLvl(0)
{
}

Autocode::~Autocode()
{
    this->flush();
}

void Autocode::increaseIndent()
{
    ++mIndentLvl;
}

void Autocode::decreaseIndent()
{
    if (mIndentLvl > 0)
    {
        --mIndentLvl;
    }
}

void Autocode::operator()()
{
    (*this)("");
}

void Autocode::flush()
{
    mOs.flush();
}

String& Autocode::format(String& kStr)
{
    return kStr;
}
