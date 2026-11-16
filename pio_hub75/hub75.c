#include "game_of_life.h"

static bool cell_exists(uint16_t x, uint16_t y) { 
    return (x < 32 && y < 64) && (x >= 0 && y >= 0); 
}

static uint8_t count_diagonal_live_neighbors(uint16_t x, uint16_t y) { 
    count = 0; 
    // upper left diagonal 
    if (cell_exists(x-1, y-1)) count += cell_states[x-1][y-1]; 
    // upper right diagonal 
    if (cell_exists(x-1, y+1)) count += cell_states[x-1][y+1];
    // lower left diagonal
    if (cell_exists(x+1, y-1)) count += cell_states[x+1][y-1]; 
    // lower right diagonal 
    if (cell_exists(x+1, y+1)) count += cell_states[x+1][y+1]; 

    return count; 
}

static uint8_t count_left_right_live_neighbors(uint16_t x, uint16_t y) { 
    count = 0; 
    // left neighbor
    if (cell_exists(x, y-1)) count += cell_states[x][y-1]; 
    // right neighbor
    if (cell_exists(x, y+1)) count += cell_states[x][y+1]; 

    return count; 
}

static uint8_t count_up_down_live_neighbors(uint16_t x, uint16_t y) { 
    count = 0; 
    // upstairs neighbor 
    if (cell_exists(x-1, y)) count += cell_states[x-1][y]; 
    // downstairs neighbor
    if (cell_exists(x+1, y)) count += cell_states[x+1][y]; 

    return count; 
}

static uint8_t count_cell_live_neighbors(uint16_t x, uint16_t y) { 
    uint8_t live = 0; 
    if (!cell_exists(x, y));
    else { 
        live += count_diagonal_live_neighbors(x, y); 
        live += count_left_right_live_neighbors(x, y); 
        live += count_up_down_live_neighbors(x, y); 
    }
    return live;
}

static bool cell_is_live(uint16_t x, uint16_t y) { 
    if (cell_exists(x, y)) return cell_states[x][y]; 
    else return false; 
}

static void cell_mark_dead(uint16_t x, uint16_t y) { 
    if (cell_exists(x, y)) cell_states[x][y] = 0; 
}

static void cell_mark_live(uint16_t x, uint16_t y) { 
    if (cell_exists(x, y)) cell_states[x][y] = 1; 
}

static void board_init(void) { 
    for (int i = 0; i < HEIGHT; i++) { 
        for (int j = 0; j < WIDTH; j++) { 
            // randomness == chance of setting a cell to 1 
            randomness = get_rand_32()%100;
            if (celsius < 70) { 
                cell_states[i][j] = (randomness < 30) ? 1 : 0; 
            }
            else if (celsius > 70 && celsius < 80)  { 
                cell_states[i][j] = (randomness < 40) ? 1 : 0; 
            }
            else if (celsius > 80 && celsius < 90)  { 
                cell_states[i][j] = (randomness < 50) ? 1 : 0; 
            } 
            else if (celsius > 90 && celsius < 100) { 
                cell_states[i][j] = (randomness < 60) ? 1 : 0; 
            } 
            else { 
                cell_states[i][j] = 0; 
            }
        }
    }
}

static void generate(void) { 
    for (uint16_t i = 0; i < HEIGHT; i++) { 
        for (uint16_t j = 0; j < WIDTH; j++) { 
            if (cell_is_live(i, j)) { 
                count = count_cell_live_neighbors(i, j); 
                if (count == 2 || count == 3) { } // stays alive, do nothing  
                else cell_mark_dead(i, j); 
            }
            else { 
                count = count_cell_live_neighbors(i, j); 
                if (count == 3) { cell_mark_live(i, j); } 
            }
        }
    }
}

// In the previous_generations array, each chunk of 64 32-bit integers 
// represents a single animation frame/board of cell states. Here, we're 
// saving the current state before overwriting it with the new values from 
// the next generation. 
static void save_previous_generation(void) {
    saved_generations = (saved_generations +1)%MAX_GENS;
    for (int i = 0; i < HEIGHT; i++) {
        uint32_t upper = 0; 
        uint32_t lower = 0;  
        for (int j = 0; j < WIDTH; j++) {
            // This is a single 512-bit row of cell_states, split into two 32-bit integers 
            if (j < 32) upper |= cell_states[i][j] == 1 ? (UINT32_C(1) << (31-j)) : 0;
            else        lower |= cell_states[i][j] == 1 ? (UINT32_C(1) << (63-j)) : 0;
        }
        size_t idx = ((size_t)((saved_generations-1)*HEIGHT + i)*2); 
        previous_generations[idx]   = upper; 
        previous_generations[idx+1] = lower; 
    }
}

static void restore_previous_generation(void) { 
    saved_generations = ((saved_generations-1 > 0) ? (saved_generations -1) : 0); 
    for (int i = 0; i < HEIGHT; i++) { 
        size_t idx = ((size_t)((saved_generations)*HEIGHT + i)*2); 
        uint32_t upper = previous_generations[idx]; 
        uint32_t lower = previous_generations[idx+1]; 
        uint x = 0; 
        for (int bit = 31; bit >= 0; bit--) { 
            cell_states[i][x++] = (upper >> bit) & 1u; 
        }
        for (int bit = 31; bit >= 0; bit--) { 
            cell_states[i][x++] = (lower >> bit) & 1u;
        }
    }
    prev_voltage = voltage;
}

static inline uint32_t gamma_correct_565_888(uint16_t pix) {
    uint32_t r_gamma = pix & 0xf800u;
    r_gamma *= r_gamma;
    uint32_t g_gamma = pix & 0x07e0u;
    g_gamma *= g_gamma;
    uint32_t b_gamma = pix & 0x001fu;
    b_gamma *= b_gamma;
    return (b_gamma >> 2 << 16) | (g_gamma >> 14 << 8) | (r_gamma >> 24 << 0);
}

static void hub75_matrix_init(void) { 
    data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, DATA_BASE_PIN, CLK_PIN);
    hub75_row_program_init(pio, sm_row, row_prog_offs, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);
}

static void display_frame(void) {

     // draw_frame 
    for (uint16_t rowsel = 0; rowsel < (1 << ROWSEL_N_PINS); ++rowsel) {
        for (uint16_t x = 0; x < WIDTH; ++x) {
            volatile uint16_t px_upper = (cell_is_live(rowsel, x) ? CELL_ALIVE_565 : CELL_DEAD_565);
            gc_row[0][x] = gamma_correct_565_888(px_upper); 
            volatile uint16_t row = ((1u << ROWSEL_N_PINS) + rowsel); 
            volatile uint16_t px_lower = cell_is_live(row, x) ? CELL_ALIVE_565 : CELL_DEAD_565; 
            gc_row[1][x] = gamma_correct_565_888(px_lower); 
        }
        for (int bit = 0; bit < 8; ++bit) {
            hub75_data_rgb888_set_shift(pio, sm_data, data_prog_offs, bit);
            for (int x = 0; x < WIDTH; ++x) {
                pio_sm_put_blocking(pio, sm_data, gc_row[0][x]);
                pio_sm_put_blocking(pio, sm_data, gc_row[1][x]);
            }
            // Dummy pixel per lane
            pio_sm_put_blocking(pio, sm_data, 0);
            pio_sm_put_blocking(pio, sm_data, 0);
            // SM is finished when it stalls on empty TX FIFO
            hub75_wait_tx_stall(pio, sm_data);
            // Also check that previous OEn pulse is finished, else things can get out of sequence
            hub75_wait_tx_stall(pio, sm_row);

            // Latch row data, pulse output enable for new row.
            pio_sm_put_blocking(pio, sm_row, rowsel | (100u * (1u << bit) << 5));
        }
    } 
}

/* 
    Get a voltage reading from the potentiometer. 
    @param N/A 
    @return N/A
*/
void read_pot(void) {
    adc_select_input(2); 
    const float conversion_factor = V_FS / (1 << 12); 
    uint16_t result = adc_read(); 
    voltage = result*conversion_factor; 
    if ((voltage - prev_voltage) > 0.25) { 
        save_previous_generation();
        next_generation = true; 
    }
    else if ((voltage - prev_voltage) < -0.25) { 
        restore_previous_generation(); 
    } 
}

/* 
    Initialize the potentiometer. 
    @param N/A
    @return N/A
*/
void pot_init(void) { 
    adc_init(); 
    adc_gpio_init(ADC_POT_GPIO_PIN); 
    read_pot(); 
}


/* 
    Get a temperature reading from the thermistor. 
    @param N/A 
    @return N/A
*/
void read_thermistor(void) {
    adc_select_input(1);  
    const float conversion_factor = V_FS / (1 << 12); 
    uint16_t result = adc_read(); 
    celsius = 1 / (log(1 / (ADC_FULL_SCALE / result - 1)) / BETA + 1.0/298.15) - 273.15; 
}

/* 
    Initialize the thermistor. 
    @param N/A
    @return N/A
*/
void thermistor_init(void) { 
    adc_gpio_init(ADC_THR_GPIO_PIN);
    read_thermistor(); 
}

int main() {
    stdio_init_all();
    pot_init();   
    thermistor_init(); 
    //board_init();
    hub75_matrix_init(); 

    while (1) {
        display_frame();
        if (next_generation) {
            prev_voltage = voltage;  
            generate(); 
            next_generation = false; 
        } 
        read_pot();
        read_thermistor();
    }

}
