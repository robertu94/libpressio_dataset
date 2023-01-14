#include <libpressio_dataset.h>
#include <libpressio.h>

int main(int argc, char *argv[])
{
    //we don't need to test everything twice, just ensure that this compiles and links with a C compiler
    struct pressio* library = pressio_instance();
    struct pressio_dataset_loader* loader =pressio_get_dataset_loader(library, "io_loader");
    pressio_dataset_loader_free(loader);
    
    return 0;
}
