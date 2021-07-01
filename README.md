## Notes

- Read the installation instructions. It's very easy to break your game if you don't.
- I wouldn't recommend you to play on VAC secured servers with this. You shouldn't get banned but don't do it just in case.
- This has been only tested properly on latest 1.6 and Condition Zero builds from Steam. This should work fine on older builds but if it doesn't, too bad.
- Some pirated clients (warzone) come with a program called gtlib which starts bitching when csldr is installed. To avoid this, csldr will remove GTLib.asi automatically when the game is launched. I'm not sure if this has any consequences but it has worked for me so far.

## Installation

1. Download Windows binary from the [releases page](https://github.com/mikkokko/csldr/releases) or compile it yourself
2. Navigate to "cstrike/cl_dlls" (or "czero/cl_dlls" if you're playing Condition Zero)
3. Rename the existing client.dll to client_orig.dll (On Linux, client.so to client_orig.so)
4. Play

## Other stuff

- To get bone controlled camera movement working, make a bone called "camera" and make an attachment on it. It sucks to rely on bone name but studiomdl doesn't support attachment names even though the format supports them.
- You can make hand bodygroups change depending on what team you're in by naming the bodygroup so that is starts with "arms".
- Inspect sequence indices are hardcoded for compatibility, it's always the sequence after the last default sequence. Check inspectAnims array in inspect.c if in doubt.

## Cvar/command list

        camera_movement_scale
        cl_bob_lower_amt
        cl_bobamt_lat
        cl_bobamt_vert
        cl_use_new_bob
        fov_horplus
        fov_lerp
        lookat
        mirror_shell
        viewmodel_fov
        viewmodel_lag_scale
        viewmodel_lag_speed
        viewmodel_offset_x
        viewmodel_offset_y
        viewmodel_offset_z
        viewmodel_shift
        xhair_color_b
        xhair_color_g
        xhair_color_r
        xhair_dot
        xhair_enable
        xhair_gap
        xhair_pad
        xhair_size
        xhair_t
        xhair_thick
