#pragma once

class Logging
{
private:
    Logging();
    ~Logging();
public:
    static Logging& GetInstance();
};
