#include <memory>
#include "tools.h"
//#include "cast.h"
#include "geo.h"
#include "nppm.h"
#include "recipe_resolver.h"
#include "recipe_validator.h"
// #include <filesystem>   C++17
#include <unistd.h>
#include <sys/stat.h>

//extern struct cmr_img_stich_record_camraLR_on_off a_cj_xyz;
extern running_mode_t el_state;

#include "cast.h"
namespace ct{
    extern Cast *ing_cast ;
}
Nppm *obj_npp;
int main(int argc, char *argv[])
{
    if (argc >= 2 && std::string(argv[1]) == "--check-recipe") {
        if (argc < 3 || argv[2] == nullptr) {
            std::cerr << "Usage: " << argv[0] << " --check-recipe <recipe.cast|recipe.yml|recipe-name>" << std::endl;
            return 2;
        }

        char *check_argv[] = {argv[0], argv[2], nullptr};
        ResolvedRecipe resolved = resolve_recipe(2, check_argv);
        validate_recipe_file_or_exit(resolved.recipe_path);

        std::cout << "Recipe check passed." << std::endl;
        std::cout << "Resolved recipe: " << resolved.recipe_path << std::endl;
        if (!resolved.metadata_root.empty()) {
            std::cout << "Metadata root: " << resolved.metadata_root << std::endl;
        }
        return 0;
    }

    //el_state.util_media_type = ING_STAT::MediaType::IMAGE_FILE;
    el_state.util_system_state = ING_STAT::SystemState::NPPM; //CAR_VIEW ; //
    el_state.recordState = ING_STAT::RecordOnOff::OFF;
    /*
    cout <<"into first line of running recipe" <<  endl ;
    char buf[256];
    utl::term::get_string_q(buf, 1024); //first line of running recipe
    cout <<"content of the first line of running recipe" << buf << endl ;
    */

    //    Nppm *obj_npp;
    //    obj_npp = new Nppm(argc, argv);
    //https://stackoverflow.com/questions/2404115/is-auto-ptr-deprecated
    std::unique_ptr<Nppm> nppm_obj(new Nppm(argc, argv));
    nppm_obj->run();
    //    delete obj_npp;
}
