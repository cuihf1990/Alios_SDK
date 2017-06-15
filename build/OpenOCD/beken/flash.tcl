set MODE_STD     0
set MODE_DUAL    1
set MODE_QUAD    2

set FLASH_BASE                           (0x00803000)

set REG_FLASH_OPERATE_SW                 [expr $FLASH_BASE + 0 * 4]
set ADDR_SW_REG_POSI                     (0)
set ADDR_SW_REG_MASK                     (0xFFFFFF)
set OP_TYPE_SW_POSI                      (24)
set OP_TYPE_SW_MASK                      (0x1F)
set OP_SW                                [expr 0x01 << 29]
set WP_VALUE                             [expr 0x01 << 30]
set BUSY_SW                              [expr 0x01 << 31]

set REG_FLASH_DATA_SW_FLASH              [expr $FLASH_BASE + 1 * 4]

set REG_FLASH_DATA_FLASH_SW              [expr $FLASH_BASE + 2 * 4]

set REG_FLASH_RDID_DATA_FLASH            [expr $FLASH_BASE + 4 * 4]

set REG_FLASH_SR_DATA_CRC_CNT            [expr $FLASH_BASE + 5 * 4]
set SR_DATA_FLASH_POSI                   (0)
set SR_DATA_FLASH_MASK                   (0xFF)
set CRC_ERROR_COUNT_POSI                 (8)
set CRC_ERROR_COUNT_MASK                 (0xFF)
set DATA_FLASH_SW_SEL_POSI               (16)
set DATA_FLASH_SW_SEL_MASK               (0x07)
set DATA_SW_FLASH_SEL_POSI               (19)
set DATA_SW_FLASH_SEL_MASK               (0x07)

set REG_FLASH_CONF                       [expr $FLASH_BASE + 7 * 4]
set FLASH_CLK_CONF_POSI                  (0)
set FLASH_CLK_CONF_MASK                  (0x0F)
set MODEL_SEL_POSI                       (4)
set MODEL_SEL_MASK                       (0x1F)
set FWREN_FLASH_CPU                      [expr 0x01 << 9]
set WRSR_DATA_POSI                       (10)
set WRSR_DATA_MASK                       (0xFFFF)
set CRC_EN                               [expr 0x01 << 26]

set FLASH_OPCODE_WREN    1
set FLASH_OPCODE_WRDI    2
set FLASH_OPCODE_RDSR    3
set FLASH_OPCODE_WRSR    4
set FLASH_OPCODE_READ    5
set FLASH_OPCODE_RDSR2   6
set FLASH_OPCODE_WRSR2   7
set FLASH_OPCODE_PP      12
set FLASH_OPCODE_SE      13
set FLASH_OPCODE_BE1     14
set FLASH_OPCODE_BE2     15
set FLASH_OPCODE_CE      16
set FLASH_OPCODE_DP      17
set FLASH_OPCODE_RFDP    18
set FLASH_OPCODE_RDID    20
set FLASH_OPCODE_HPM     1
set FLASH_OPCODE_CRMR    22
set FLASH_OPCODE_CRMR2   23

proc read_reg { addr } {
    mem2array memar 32 $addr 1
    return $memar(0)
}

proc write_reg { addr val } {
    set memar(0) $val
    array2mem memar 32 $addr 1
}

proc flash_set_line_mode_2 { } {
    #UINT32 value;
    #value = REG_READ(REG_FLASH_CONF);
    set value [read_reg $::REG_FLASH_CONF]
    #value &= ~(MODEL_SEL_MASK << MODEL_SEL_POSI);
    set value [expr $value & (~($::MODEL_SEL_MASK << $::MODEL_SEL_POSI))]
    #REG_WRITE(REG_FLASH_CONF, value);
    write_reg $::REG_FLASH_CONF $value

    #value |= ((MODE_DUAL & MODEL_SEL_MASK) << MODEL_SEL_POSI);
    set value [expr $value | (($::MODE_DUAL & $::MODEL_SEL_MASK) << $::MODEL_SEL_POSI)]
    #REG_WRITE(REG_FLASH_CONF, value);
    write_reg $::REG_FLASH_CONF $value
}

proc flash_set_clk { conf } {
    #value = REG_READ(REG_FLASH_CONF);
    set value [read_reg $::REG_FLASH_CONF]
    #value &= ~(FLASH_CLK_CONF_MASK << FLASH_CLK_CONF_POSI);
    set value [expr $value & (~($::FLASH_CLK_CONF_MASK << $::FLASH_CLK_CONF_POSI))]
    #value |= (clk_conf << FLASH_CLK_CONF_POSI);
    set value [expr $value | ($conf << $::FLASH_CLK_CONF_POSI)]
    #REG_WRITE(REG_FLASH_CONF, value);    
    write_reg $::REG_FLASH_CONF $value
}

proc set_flash_unprotect { } {
    # unsigned int temp0;
    # UINT8 bit_QE  = 0;
    set bit_QE 0

    # while(REG_READ(REG_FLASH_OPERATE_SW) & BUSY_SW);
    while { [expr [read_reg $::REG_FLASH_OPERATE_SW] & $::BUSY_SW] } { }

    # temp0 = REG_READ(REG_FLASH_CONF); //����WRSR Status data
    set temp0 [read_reg $::REG_FLASH_CONF]
    # temp0 &= 0xfffe0fff;  // set [BP4:BP0] = 0
    set temp0 [expr $temp0 & 0xfffe0fff]
    # *((volatile UINT32 *)(REG_FLASH_CONF)) = ((temp0 &  FLASH_CLK_CONF_MASK)| (bit_QE << 19)| (0x10000 << WRSR_DATA_POSI));  // unprotect all sectors
    set temp0 [expr (($temp0 & $::FLASH_CLK_CONF_MASK) | ($bit_QE << 19) | (0x10000 << $::WRSR_DATA_POSI))]
    write_reg $::REG_FLASH_CONF $temp0

    # //Start WRSR
    # temp0 = *((volatile UINT32 *)(REG_FLASH_OPERATE_SW));
    set temp0 [read_reg $::REG_FLASH_OPERATE_SW]
    # *((volatile UINT32 *)(REG_FLASH_OPERATE_SW)) = ((temp0 & ADDR_SW_REG_MASK)| (FLASH_OPCODE_WRSR2 << OP_TYPE_SW_POSI)| OP_SW| WP_VALUE); // make WP equal 1 not protect SRP
    set temp0 [expr (($temp0 & $::ADDR_SW_REG_MASK) | ($::FLASH_OPCODE_WRSR2 << $::OP_TYPE_SW_POSI)| $::OP_SW | $::WP_VALUE)]
    # while(REG_READ(REG_FLASH_OPERATE_SW) & BUSY_SW);
    while { [expr [read_reg $::REG_FLASH_OPERATE_SW] & $::BUSY_SW] } { }
}

proc flash_write_enable { } {
    set value [read_reg $::REG_FLASH_CONF]
    set value [expr $value & (~$::CRC_EN)] 
    write_reg $::REG_FLASH_CONF $value    

    set value [read_reg $::REG_FLASH_CONF]
    set value [expr $value & (~($::WRSR_DATA_MASK<<$::WRSR_DATA_POSI))]  
    write_reg $::REG_FLASH_CONF $value  

    set value [read_reg $::REG_FLASH_CONF]
    set value [expr $value | $::FWREN_FLASH_CPU] 
    write_reg $::REG_FLASH_CONF $value  

    set value [read_reg $::REG_FLASH_OPERATE_SW]
    set value [expr $value & (~($::OP_TYPE_SW_MASK<<$::OP_TYPE_SW_POSI))] 
    set value [expr $value | (0x07<<$::OP_TYPE_SW_POSI)|$::OP_SW|$::WP_VALUE] 
    write_reg $::REG_FLASH_OPERATE_SW $value  
    while { [expr [read_reg $::REG_FLASH_OPERATE_SW] & $::BUSY_SW] } { }
}

proc flash_init { } {
    #while(REG_READ(REG_FLASH_OPERATE_SW) & BUSY_SW);
    while { [expr [read_reg $::REG_FLASH_OPERATE_SW] & $::BUSY_SW] } { }
    #flash_set_line_mode(2);
    flash_set_line_mode_2
    #flash_set_clk(5);  // 60M
    flash_set_clk 5

    flash_write_enable
}

proc flash_erase_sector { addr } {
    #UINT32 value;
    #while(REG_READ(REG_FLASH_OPERATE_SW) & BUSY_SW);
    while { [expr [read_reg $::REG_FLASH_OPERATE_SW] & $::BUSY_SW] } { }

    #flash_set_line_mode(0);
    #//set_flash_protect(0);
    #set_flash_unprotect();
    #while(REG_READ(REG_FLASH_OPERATE_SW) & BUSY_SW);
    #value = REG_READ(REG_FLASH_OPERATE_SW);
    set value [read_reg $::REG_FLASH_OPERATE_SW]
    #value = ((addr << ADDR_SW_REG_POSI)| (FLASH_OPCODE_SE << OP_TYPE_SW_POSI)| OP_SW | (value & WP_VALUE));
    set value [expr (($addr << $::ADDR_SW_REG_POSI)| ($::FLASH_OPCODE_SE << $::OP_TYPE_SW_POSI)| $::OP_SW | ($value & $::WP_VALUE))]
    #REG_WRITE(REG_FLASH_OPERATE_SW, value);
    write_reg $::REG_FLASH_OPERATE_SW $value
    # while(REG_READ(REG_FLASH_OPERATE_SW) & BUSY_SW);
    while { [expr [read_reg $::REG_FLASH_OPERATE_SW] & $::BUSY_SW] } { }
    # set_flash_protect(1);
    # flash_set_line_mode(2);    
}

proc flash_erase { addr size } {
    while { $size } {
        flash_erase_sector $addr
        set addr [expr $addr + 0x1000]
        set size [expr $size - 0x1000]
    }
}

proc flash_init_x { } {
    # Flash data to CPU CRC disable
    puts ">>>>>>>> Flash data to CPU CRC disable"
    mem2array tmp_arr 32 0x0080301c 1
    set reg_val [expr ($tmp_arr(0))&(~(1<<26))]
    mww 0x0080301c $reg_val

    # Flash status register data to be written
    puts ">>>>>>>> Flash status register data to be written"
    mem2array tmp_arr 32 0x0080301c 1
    set reg_val [expr ($tmp_arr(0))&(~(0xffff<<10))]
    mww 0x0080301c $reg_val    

    # CPU data writting enable
    puts ">>>>>>>> CPU data writting enable"
    mem2array tmp_arr 32 0x0080301c 1
    set reg_val [expr ($tmp_arr(0))|(1<<9)]
    mww 0x0080301c $reg_val

    # Flash operation command : WRSR2
    puts ">>>>>>>> Flash operation command : WRSR2"
    mem2array tmp_arr 32 0x00803000 1
    set reg_val $tmp_arr(0)
    # clr op_type_sw
    set reg_val [expr $reg_val&(~(0x1f<<24))]
    # op_sw, wp_value
    set reg_val [expr $reg_val|((0x07<<24)|(1<<29)|(1<<30))]
    mww 0x00803000 $reg_val
    sleep 1000
    set ready 1
    puts ">>>>>>>> Waitting for operation completed..."
    while { $ready } {
        mem2array tmp_arr 32 0x00803000 1
        set reg_val $tmp_arr(0)
        set ready [expr $reg_val&(0x01<<31)]
    }
    puts ">>>>>>>> Operation completed!"

    # Flash operation command : CE
    puts ">>>>>>>> Flash operation command : CE"
    mem2array tmp_arr 32 0x00803000 1
    set reg_val $tmp_arr(0)
    # clr op_type_sw
    set reg_val [expr $reg_val&(~(0x1f<<24))]
    # op_sw, wp_value
    set reg_val [expr $reg_val|((0x10<<24)|(1<<29)|(1<<30))]
    mww 0x00803000 $reg_val
    sleep 1000
    set ready 1
    puts ">>>>>>>> Waitting for operation completed..."
    while { $ready } {
        mem2array tmp_arr 32 0x00803000 1
        set reg_val $tmp_arr(0)
        set ready [expr $reg_val&(0x01<<31)]
    }
    puts ">>>>>>>> Operation completed!"
}