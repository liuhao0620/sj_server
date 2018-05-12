#include "fgs.h"
#include "simple_logger.h"
#include "common_def.h"

int main(int argc, char **argv)
{
    char * cfg_file = NULL;
    if (argc >= 2)
    {
        cfg_file = argv[1];
    }
    if (!fgs::Init(cfg_file))
    {
        ASSERT(false && "Init failed!!!");
        return -1;
    }
    while (true)
    {
        //主循环
        fgs::Update();
    }
    fgs::Close();
    return 0;
}
