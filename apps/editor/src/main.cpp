#include <editor/editor_engine.h>

int main()
{
    editor::editor_engine instance({
        .appName = "RatsEditor"
    });
    if (!instance.start())
    {
        engine::log::fatal("[Editor] Some error happened!");
        return -1;
    }
    return 0;
}
