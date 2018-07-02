#include "xd75.h"

/* Layer shorthand */
    #define _QW  0
    #define _GM  1
    #define _DV  2
    #define _NUM 3
    #define _SYM 4
    #define _BSP 5
    #define _SPC 6
    #define _ENT 7

/*********************************************************************
                            TAP DANCE
*********************************************************************/
    enum {
      _ESCQUIT = 0,
      _GRVQUIT
    };

    qk_tap_dance_action_t tap_dance_actions[] = {
      [_ESCQUIT]  = ACTION_TAP_DANCE_DOUBLE( KC_ESC, LCTL( LSFT( KC_Q ) ) ),
      [_GRVQUIT]  = ACTION_TAP_DANCE_DOUBLE( KC_GRV, LCTL( LSFT( KC_Q ) ) )
    };

/*********************************************************************
                             MACROS
*********************************************************************/
    // MODE 2-5,   breathing
    // MODE 6-8,   rainbow mood
    // MODE 9-14,  rainbow swirl
    // MODE 15-20, snake
    // MODE 21-23, knight
    // MODE 24,    christmas
    // MODE 25-34, static gradient

    // Constants
        #define LAYER_MIN 0
        #define LAYER_MAX 2
        #define TEN_KEY_RGB_MODE 17
        #define SYMBOLS_RGB_MODE 23
        #define CAPS_RGB_MODE 5
        #define TIMER_LENGTH 150

    // Variables
        int current_layer = 0;
        int previous_layer = 0;

        uint16_t caps_lock_timer;
        bool caps_is_on = false;

        uint16_t num_timer;
        bool num_is_on = false;

        uint16_t alt_timer;
        bool alt_is_down = false;

        const uint16_t rgb_mode_map[] = {
            [0]   = 1,
            [1]   = 27,
            [2]   = 28,
            [3]   = 1,
            [4]   = 1,
            [5]   = 1,
            [6]   = 1,
            [7]   = 1,
            [8]   = 1,
            [9]   = 1,
            [10]  = 1,
            [11]  = 1,
            [12]  = 1,
            [13]  = 1,
            [14]  = 1,
            [15]  = 1
        };

    enum custom_keycodes {
        // Location List
        LLPRV = SAFE_RANGE,
        LLNXT,

        // Quickfix List
        QLPRV,
        QLNXT,

        // Switch buffers
        BFPRV,
        BFNXT,

        // Tmux windows
        TMPRV,
        TMNXT,
        TMMAX,

        // Custom Commands
        NXTLYR,
        TENKEY,
        SYMBOL,
        CPSLCK,
        SG_ALT, // MT( MOD_LALT, LGUI( KC_GRV ) )
        DYNAMIC_MACRO_RANGE // Must be last
    };

    #include "dynamic_macro.h"

    bool process_record_user(uint16_t keycode, keyrecord_t *record) {
        if ( !process_record_dynamic_macro( keycode, record ) ) {
            return false;
        }

        switch( keycode ) {
            // Vim Commands
                case LLPRV:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING("[l");
                        return false;
                    }
                }
                case LLNXT:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING("]l");
                        return false;
                    }
                }
                case QLPRV:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING("[q");
                        return false;
                    }
                }
                case QLNXT:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING("]q");
                        return false;
                    }
                }
                case BFPRV:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING("[b");
                        return false;
                    }
                }
                case BFNXT:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING("]b");
                        return false;
                    }
                }

            // Tmux Commands
                case TMPRV:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING(SS_LCTRL("a")"p");
                        return false;
                    }
                }
                case TMNXT:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING(SS_LCTRL("a")"n");
                        return false;
                    }
                }
                case TMMAX:
                {
                    if ( record->event.pressed ) {
                        SEND_STRING(SS_LCTRL("a")"z");
                        return false;
                    }
                }

            // Custom Commands
                case NXTLYR:
                {
                    if ( record->event.pressed )
                    {
                        current_layer++;
                        if ( current_layer > LAYER_MAX )
                            current_layer = LAYER_MIN;

                        layer_off( previous_layer );
                        layer_on( current_layer );

                        rgblight_mode( rgb_mode_map[ current_layer ] );
                        previous_layer = current_layer;

                        return false;
                    }
                }
                case TENKEY:
                {
                    if ( record->event.pressed )
                    {
                        num_timer = timer_read();
                        layer_on( _NUM );
                        rgblight_mode( TEN_KEY_RGB_MODE );
                    }
                    else
                    {
                        if ( timer_elapsed( num_timer ) < TIMER_LENGTH )
                        {
                            if ( num_is_on ) {
                                num_is_on = false;
                            } else {
                                num_is_on = true;
                                return false;
                            }
                        }

                        if ( caps_is_on )
                            rgblight_mode( CAPS_RGB_MODE );
                        else
                            rgblight_mode( rgb_mode_map[ current_layer ] );

                        layer_off( _NUM );
                    }
                    return false;
                }
                case SYMBOL:
                {
                    if ( record->event.pressed )
                    {
                        layer_on( _SYM );
                        rgblight_mode( SYMBOLS_RGB_MODE );
                    }
                    else
                    {
                        layer_off( _SYM );

                        if ( caps_is_on )
                            rgblight_mode( CAPS_RGB_MODE );
                        else
                            rgblight_mode( rgb_mode_map[ current_layer ] );
                    }
                    return false;
                }
                case CPSLCK:
                {
                    if ( record->event.pressed )
                    {
                        caps_lock_timer = timer_read();
                        SEND_STRING( SS_DOWN( X_LCTRL ) );
                    }
                    else
                    {
                        SEND_STRING( SS_UP( X_LCTRL ) );

                        if ( timer_elapsed( caps_lock_timer ) < TIMER_LENGTH )
                        {
                            if ( caps_is_on ) {
                                if ( num_is_on )
                                    rgblight_mode( TEN_KEY_RGB_MODE );
                                else
                                    rgblight_mode( rgb_mode_map[ current_layer ] );
                                backlight_level( 0 );

                                SEND_STRING( SS_TAP( X_CAPSLOCK ) );

                                caps_is_on = false;
                            } else {
                                rgblight_mode( CAPS_RGB_MODE );
                                backlight_level( 255 );

                                SEND_STRING( SS_TAP( X_CAPSLOCK ) );

                                caps_is_on = true;
                            }
                        }
                    }
                    return false;
                }
                case SG_ALT:
                {
                    if ( record->event.pressed )
                    {
                        alt_timer = timer_read();
                        SEND_STRING( SS_DOWN( X_LALT ) );
                    }
                    else
                    {
                        SEND_STRING( SS_UP( X_LALT ) );

                        if ( timer_elapsed( alt_timer ) < TIMER_LENGTH )
                        {
                            SEND_STRING( SS_LGUI("`") );
                        }
                    }

                    return false;
                }
        }

        return true;
    };

/*********************************************************************
                            KEY MAPS
*********************************************************************/
      /* BLANK
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */
         /*
         [_FN] = {
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ }}
         */

    const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
      /* QWERTY
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         | `       | 1       | 2       | 3       | 4       | 5       | backspc | trminal | delete  | 6       | 7       | 8       | 9       | 0       | equal   |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | tab_alt | q       | w       | e       | r       | t       | [       | mcrorec | ]       | y       | u       | i       | o       | p       | minus   |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | cap_ctl | a       | s       | d       | f       | g       | (       | mcroply | )       | h       | j       | k       | l       | ;       | '       |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | lshift  | z       | x       | c       | v       | b       | {       | mcrostp | }       | n       | m       | ,       | .       | /       | lshift  |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | L_NUM   | L_SYM   | alt     | super   | l_bsp   | ctl_bs  | f1      | escquit | alttab  | enter   | space   | _       | |       | \       | L_NEXT  |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */

             #define ALT MT( MOD_LALT, KC_TAB )
             #define SHIFT MT( MOD_LSFT, KC_ESC )
             #define CTLBS LCTL(KC_BSPC)
             #define ALTGRV LALT( KC_GRV )
             #define ESCQUIT TD( _ESCQUIT )
             #define TERM LGUI( KC_T )
             #define SUPGRV LGUI( KC_GRV )
             
             #define LT_BSPC LT( _BSP, KC_BSPC )
             #define LT_SPC LT( _SPC, KC_SPC )
             #define LT_ENT LT( _ENT, KC_ENT )
           
             #define MCR_REC DYN_REC_START1
             #define MCR_PLY DYN_MACRO_PLAY1
             #define MCR_STP DYN_REC_STOP

         [_QW] = {
         { KC_GRV  , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , KC_BSPC , TERM    , KC_DEL  , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , KC_EQL  },
         { ALT     , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T    , KC_LBRC , MCR_REC , KC_RBRC , KC_Y    , KC_U    , KC_I    , KC_O    , KC_P    , KC_MINS },
         { CPSLCK  , KC_A    , KC_S    , KC_D    , KC_F    , KC_G    , KC_LPRN , MCR_PLY , KC_RPRN , KC_H    , KC_J    , KC_K    , KC_L    , KC_SCLN , KC_QUOT },
         { SHIFT   , KC_Z    , KC_X    , KC_C    , KC_V    , KC_B    , KC_LCBR , MCR_STP , KC_RCBR , KC_N    , KC_M    , KC_COMM , KC_DOT  , KC_SLSH , KC_LSFT },
         { TENKEY  , SYMBOL  , KC_LALT , KC_LGUI , LT_BSPC , SG_ALT  , KC_F1   , ESCQUIT , ALTGRV  , LT_ENT  , LT_SPC  , KC_UNDS , KC_PIPE , KC_BSLS , NXTLYR  }},
     
      /* GAMING
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | Tab     |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | lctl    |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | lshift  |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         | lctl    | lctl    | lctl    | lalt    | GM_SPC  | lctl    |         | KC_ESC  |         | KC_ENT  | KC_SPC  | LT_UND  |         |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */

             #define GM_SPC LT( _BSP, KC_SPC )
             #define LT_UND LT( _ENT, KC_UNDS )

         [_GM] = {
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { KC_TAB  , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { KC_LCTL , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { KC_LSFT , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { KC_LCTL , KC_LCTL , KC_LCTL , KC_LALT , GM_SPC  , KC_LCTL , _______ , KC_ESC  , _______ , KC_ENT  , KC_SPC  , LT_UND  , _______ , _______ , _______ }},

      /* DVORAK
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | '       | ,       | .       | p       | y       |         |         |         | f       | g       | c       | r       | l       |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | a       | o       | e       | u       | i       |         |         |         | d       | h       | t       | n       | s       | /       |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | ;       | q       | j       | k       | x       |         |         |         | b       | m       | w       | v       | z       |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */

         [_DV] = {
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , KC_QUOT , KC_COMM , KC_DOT  , KC_P    , KC_Y    , _______ , _______ , _______ , KC_F    , KC_G    , KC_C    , KC_R    , KC_L    , _______ },
         { _______ , KC_A    , KC_O    , KC_E    , KC_U    , KC_I    , _______ , _______ , _______ , KC_D    , KC_H    , KC_T    , KC_N    , KC_S    , KC_SLSH },
         { _______ , KC_SCLN , KC_Q    , KC_J    , KC_K    , KC_X    , _______ , _______ , _______ , KC_B    , KC_M    , KC_W    , KC_V    , KC_Z    , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ }},

      /* 10-KEY
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         | numlock |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         | (       | 7       | 8       | 9       | )       | ^       |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         | *       | 4       | 5       | 6       | +       | enter   |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         | /       | 1       | 2       | 3       | -       | =       |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         | space   | 0       | .       | ,       |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */
     
         [_NUM] = {
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_NLCK },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_LPRN , KC_7    , KC_8    , KC_9    , KC_RPRN , KC_CIRC },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_PAST , KC_4    , KC_5    , KC_6    , KC_PPLS , KC_ENT  },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_PSLS , KC_1    , KC_2    , KC_3    , KC_PMNS , KC_EQL  },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_SPC  , KC_0    , KC_PDOT , KC_COMM , _______ , _______ }},

      /* SYMBOLS
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         | &       | *       | (       |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         | $       | %       | ^       |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         | !       | @       | #       |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         | )       |         |         |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */
     
         [_SYM] = {
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_AMPR , KC_ASTR , KC_LPRN , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_DLR  , KC_PERC , KC_CIRC , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_EXLM , KC_AT   , KC_HASH , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_RPRN , _______ , _______ , _______ , _______ }},

      /* Backspace Layer
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         | quit    | f1      | f2      | f3      | f4      | f5      | f11     |         | f12     | f6      | f7      | f8      | f9      | f10     | print   |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | cs_t    | ctl_w   | cs_tab  | ctl_tab | ctrl_t  |         |         |         | sc_l    | sc_d    | sc_u    | sc_r    |         | aprint  |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | ctrl_a  | ctl_s   | supgrv  | sup_a   | cs_v    |         |         |         | s_l     | s_d     | s_u     | s_r     | DESK_I  | caprint |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | undo    | cut     | copy    | paste   | ctl_b   |         |         |         | sa_l    | sa_d    | sa_u    | sa_r    | DESK_D  |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         | sup_esc |         |         | sup_spc |         |         |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */

             #define SUP_A LGUI( KC_A )
             #define QUIT LCTL( LSFT( KC_Q ) )
             #define SUP_SPC LGUI( KC_SPC )
             #define SUP_ESC LGUI( KC_ESC )

             #define S_L LGUI( KC_H )
             #define S_R LGUI( KC_L )
             #define S_U LGUI( KC_K )
             #define S_D LGUI( KC_J )
             #define SA_L LALT( LGUI( KC_H ) )
             #define SA_R LALT( LGUI( KC_L ) )
             #define SA_U LALT( LGUI( KC_K ) )
             #define SA_D LALT( LGUI( KC_J ) )
             #define SC_L LCTL( LGUI( KC_H ) )
             #define SC_R LCTL( LGUI( KC_L ) )
             #define SC_U LCTL( LGUI( KC_K ) )
             #define SC_D LCTL( LGUI( KC_J ) )

             #define CTL_A LCTL( KC_A )
             #define CTL_B LCTL( KC_B )
             #define CTL_P LCTL( KC_P )
             #define CTL_S LCTL( KC_S )
             #define C_T LCTL( KC_T )
             #define CTL_W LCTL( KC_W )
             #define CS_T LCTL( LSFT( KC_T ) )

             #define C_TAB LCTL( KC_TAB )
             #define CS_TAB LCTL( LSFT( KC_TAB ) )

             #define PRINT KC_PSCR
             #define APRINT LALT( KC_PSCR )
             #define CAPRINT LCTL( LALT( KC_PSCR ) )

             #define _COPY LCTL( KC_C )
             #define _PASTE LCTL( KC_V )
             #define CS_V LCTL( LSFT( KC_V ) )
             #define _CUT LCTL( KC_X )
             #define _UNDO LCTL( KC_Z )

             #define DESK_I LGUI( KC_PPLS )
             #define DESK_D LGUI( KC_PMNS )

         [_BSP] = {
         { QUIT    , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , KC_F11  , _______ , KC_F12  , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , PRINT   },
         { _______ , CS_T    , CTL_W   , CS_TAB  , C_TAB   , C_T     , _______ , _______ , _______ , SC_L    , SC_D    , SC_U    , SC_R    , _______ , APRINT  },
         { _______ , CTL_A   , CTL_S   , SUPGRV  , SUP_A   , CS_V    , _______ , _______ , _______ , S_L     , S_D     , S_U     , S_R     , DESK_I  , CAPRINT },
         { _______ , _UNDO   , _CUT    , _COPY   , _PASTE  , CTL_B   , _______ , _______ , _______ , SA_L    , SA_D    , SA_U    , SA_R    , DESK_D  , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , SUP_ESC , _______ , _______ , SUP_SPC , _______ , _______ , _______ , _______ }},

      /* Space Layer
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         | reset   |         |         | RGB_SAD | RGB_SAI |         |         |         |         |         |         |         |         |         | sleep   |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         | RGB_HUD | RGB_HUI |         |         |         |         |         | AUD_HP  | AUD_ALL |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         | RGB_N   | RGB_TOG |         |         |         | left    | down    | up      | right   |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         | RGB_VAD | RGB_VAI |         |         |         |         |         | voldown | volup   | mute    |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         | a_left  | a_rght  | c_left  | c_right |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */

             #define VOL_D RCTL( KC_P2 )
             #define VOL_U RCTL( KC_P8 )
             #define VOL_M RCTL( KC_P5 )

             #define A_LEFT LALT( KC_LEFT )
             #define A_RIGHT LALT( KC_RGHT )
             #define C_LEFT LCTL( KC_LEFT )
             #define C_RIGHT LCTL( KC_RGHT )

             #define AUD_HP LGUI( KC_PAST )
             #define AUD_ALL LGUI( KC_KP_0 )

             #define RGB_N RGB_MOD

         [_SPC] = {
         { RESET   , _______ , _______ , RGB_SAD , RGB_SAI , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_SLEP },
         { _______ , _______ , _______ , RGB_HUD , RGB_HUI , _______ , _______ , _______ , _______ , _______ , AUD_HP  , AUD_ALL , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , RGB_N   , RGB_TOG , _______ , _______ , _______ , KC_LEFT , KC_DOWN , KC_UP   , KC_RGHT , _______ , _______ },
         { _______ , _______ , _______ , RGB_VAD , RGB_VAI , _______ , _______ , _______ , _______ , _______ , VOL_D   , VOL_U   , VOL_M   , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , A_LEFT  , A_RIGHT , C_LEFT  , C_RIGHT }},

      /* Enter Layer
         .-----------------------------------------------------------------------------------------------------------------------------------------------------.
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         | QLPRV   | LLPRV   | LLNXT   | QLNXT   |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         | home    | pgdn    | pgup    | end     |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         | TMMAX   |         |         |         |         |         |         |         | BFPRV   | TMPRV   | TMNXT   | BFNXT   |         |         |
         |---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------+---------|
         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |         |
         '-----------------------------------------------------------------------------------------------------------------------------------------------------' */

         [_ENT] = {
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , QLPRV   , LLPRV   , LLNXT   , QLNXT   , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , KC_HOME , KC_PGDN , KC_PGUP , KC_END  , _______ , _______ },
         { _______ , TMMAX   , _______ , _______ , _______ , _______ , _______ , _______ , _______ , BFPRV   , TMPRV   , TMNXT   , BFNXT   , _______ , _______ },
         { _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ }}
    };
