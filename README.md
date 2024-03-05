
## Main features

- Hor+ FOV scaling for widescreen
- Separately calculated viewmodel FOV
- Adjustable viewmodel origin
- Alternative viewmodel bob from CS:GO 1.0.0.40
- Viewmodel sway/lag
- Viewmodel shifting can be disabled
- Alternative crosshair
- Bodygroup-based arm changing depending on team
- Separate hand models for viewmodels (CS:GO/GMod style)
- Client-side weapon inspecting
- Bone controlled camera movement
- Mirrored shell ejects
- FOV lerp
- Shader-based model renderer

## Notes

- Read the installation instructions. It's very easy to break your game if you don't.
- Play on VAC secured servers at your own risk.
- Software and D3D renderers are not supported.
- Some features will not work when cl_lw is set to 0 (inspecting, crosshair...).
- Other client-side mods that rely on client.dll (like some MetaHook plugins) will not not work when this is installed.

## Non-Steam builds

Some non-Steam builds of the game (notably CS WaRzOnE) come with a program called GTProtector. This causes problems with csldr like the `Find Cl/En/St modules error` pop-up or the game crashing on startup. As a workaround, csldr will remove `GTLib.asi` or `GTProtector.asi` when the game is launched. GTProtector is apparently used to protect the client from malicious servers so it might not be a good idea to play online with these builds when clsdr is installed. Using the latest Steam version of the game is recommended.

## Installation

1. Download the latest Windows or Linux binary from the [releases page](https://github.com/mikkokko/csldr/releases) or compile it from source
2. Navigate to "cstrike/cl_dlls" (or "czero/cl_dlls" if you're playing Condition Zero)
3. Rename the existing client.dll to client_orig.dll (client.so to client_orig.so on Linux)
4. Move the new client.dll into the folder
5. Play

## Shader-based model renderer

The shader-based model renderer requires OpenGL 2.0 for GLSL shaders. For GPU skinning to work, OpenGL 2.1 is required for non-square GLSL matrices and the GL_ARB_uniform_buffer_object extension is needed for UBOs. Currently the renderer doesn't have a lot going for it besides a few nice to have things:
- Improved performance in some cases
- Support for the fullbright texture flag
- Chrome textures are no longer limited to 64x64
- Support for more than 2048 vertices/normals per submodel
- External texture replacements (see [Model config files](#model-config-files))

Eventually I will add more features to it for modding purposes.

Known issues with the renderer:
- Glowshell uses model normals instead of computing custom ones
- Rendering of hulls, bboxes or bones is not supported (r_drawentities > 1)
- Having a lot of entities visible at once tanks the frame rate on some systems

The renderer can be enabled with `studio_fastpath 1` if the system supports it. To see information about the renderer's state, use `studio_info`. If you notice any issues with the renderer (models not looking as they should, worse performance, etc.) let me know by opening an issue.

## Model config files

Model config files allow you to tweak model specific options. Here is an example config file for the ak47 viewmodel (`models/v_ak47.txt`), which contains all of the available options:
```c
// subset of valve's keyvalue format
// c++ style comments are supported (//)
// strings can be quoted or unquoted (latter is preferred if the string doesn't contain spaces)

// viewmodel tweaks
mirror_shell 1 // mirror shell ejects
mirror_model 1 // mirror the viewmodel
origin "1 0 -1" // additional origin adjustment
fov_override 74 // scales accordingly to the value of viewmodel_fov (90 = no change)

// texture overrides, these only work when studio_fastpath is 1
textures
{
    ak47.bmp // embedded texture name
    {
        diffuse textures/models/v_ak47/ak47.tga
    }
}
```

The texture replacements only work when the shader-based model renderer is active. Only TGA files are supported.

Model config files can be reloaded with the `studio_config_flush` command. This will also reload texture files.

## Other stuff

- To get bone controlled camera movement working, make a bone called "camera" and make an attachment on it.
- You can make hand bodygroups change depending on what team you're in by naming the bodygroup so that is starts with "arms".
- Inspect sequence indices are hardcoded for compatibility, it's always the sequence after the last default sequence. Check inspectAnims array in inspect.c if in doubt.

## Cvars and commands

| Name | Description |
|-|-|
| camera_movement_interp | Smooths out camera movement when switching weapons. Recommended value is 0.1. Set to 0 to disable smoothing. |
| camera_movement_scale | Camera movement scale. |
| cl_bob_lower_amt | Specifies how much the viewmodel moves inwards for CS:GO style bob. |
| cl_bobamt_lat | Lateral scale for CS:GO style bob. |
| cl_bobamt_vert | Vertical scale for CS:GO style bob. |
| cl_bobstyle | 0 for default bob, 1 for old style bob and 2 for CS:GO style bob. |
| cl_rollangle | Viewmodel roll angle. |
| cl_rollspeed | Viewmodel roll speed. |
| fov_horplus | Enables Hor+ scaling for FOV. Fixes the FOV when playing with aspect ratios other than 4:3. |
| fov_lerp | FOV interpolation time in seconds. |
| lookat | Inspects the weapon if the animation is present. |
| studio_confg_flush | Reloads model config files and any files referenced by them (like textures). |
| studio_fastpath | Enables the shader-based model renderer. |
| studio_info | Prints information about the shader-based model renderer. |
| viewmodel_fov | Viewmodel FOV. |
| viewmodel_hands | Specifies an external hand model, for example "v_hands.mdl". Can be disabled with an empty string (""). |
| viewmodel_lag_style | Viewmodel sway style. 0 is off, 1 is HL2 style and 2 is CS:S/CS:GO style. |
| viewmodel_lag_scale | Scale of the viewmodel sway (HL2 sway only). |
| viewmodel_lag_speed | Speed of the viewmodel sway. |
| viewmodel_offset_x | Viewmodel's x offset. |
| viewmodel_offset_y | Viewmodel's y offset. |
| viewmodel_offset_z | Viewmodel's z offset. |
| viewmodel_shift | Fixes the viewmodel shift when looking up and down. 1 disables the shift without fixing the viewmodel position, 2 disables the shift and fixes the viewmodel position. |
| xhair_alpha | Crosshair's transparency (0-1). |
| xhair_color_b | Crosshair color's blue value (0-1). |
| xhair_color_g | Crosshair color's green value (0-1). |
| xhair_color_r | Crosshair color's red value (0-1). |
| xhair_dot | Enables crosshair dot. |
| xhair_dynamic_scale | Scale of the dynamic crosshair movement. Set to 0 to disable the dynamic crosshair, 1 for default amount of movement. |
| xhair_gap_useweaponvalue | Makes the crosshair gap scale depend on the active weapon. |
| xhair_enable | Enables enhanced crosshair. |
| xhair_gap | Space between crosshair's lines. |
| xhair_pad | Border around crosshair. |
| xhair_size | Crosshair size. |
| xhair_t | Enables T-shaped crosshair. |
| xhair_thick | Crosshair thickness. |
