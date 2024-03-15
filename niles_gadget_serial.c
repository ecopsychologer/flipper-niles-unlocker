#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <furi_hal_serial_types.h>

#define UART_BAUDRATE 38400
#define UART_TX_PIN LL_GPIO_PIN_13
#define UART_RX_PIN LL_GPIO_PIN_14

// Status messages
const char* waiting_for_connection = "Waiting for connection";
const char* connected = "Connected";
const char* transmitting = "Transmitting";
const char* transmitted = "Transmitted";

const FuriHalSerialId uart_id = FuriHalSerialIdUsart;

// Function declarations
void update_display(const char* message);
bool wait_for_ok_button();

void uart_send_data(FuriHalSerialId uart_id, const char* data, size_t size) {
    furi_hal_gpio_init(UART_TX_PIN, LL_GPIO_MODE_OUTPUT, LL_GPIO_PULL_NO, UART_BAUDRATE);

    // Send data
    furi_hal_gpio_write(UART_TX_PIN, data, size);
}

int32_t application_main(void* p) {
    UNUSED(p);

    const char* trigger = "\x1b"; // Escape character
    size_t trigger_length = 4;
    const char* response =
        "...# Bam IR Capture Unit Ver 0.35      31Oct05..# Firmware Downloadable...";
    size_t response_length = 74;
    uint8_t rx_byte;

    while(1) {
        update_display(waiting_for_connection);
        uint8_t rx_buffer[trigger_length];
        size_t bytes_read = furi_hal_uart_read(
            uart_id, rx_buffer, sizeof(rx_buffer), 500); // Adjust timeout as needed
        if(rx_buffer[0] == trigger[0]) {
            update_display(connected);
            if(rx_buffer == trigger) {
                update_display(transmitting);
                uart_send_data(uart_id, response, response_length);
                furi_delay_ms(250);
                update_display(transmitted);
                furi_delay_ms(250);
                /*
                uint8_t timer = 0;
                if(wait_for_ok_button() || (timer == 3)) {
                    // Loop back to the beginning
                    furi_delay_ms(1000);
                    timer++;
                    continue;
                }
                */
            }
        }
    }

    return 0;
}



void update_display(const char* message) {
    // Obtain the GUI API context
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, update_display_draw_callback, (void*)message);
    view_port_enabled_set(view_port, true);

    while(!view_port_render(view_port)) {
        // Waiting for the next frame
        osMessageQueueGet(...); // Adjust according to your message loop
    }

    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
}

// Draw callback function that uses the message
static void update_display_draw_callback(Canvas* canvas, void* ctx) {
    const char* message = (const char*)ctx;
    // Ensure the canvas is clean before drawing
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, message);
}

/*
bool wait_for_ok_button() {
    // Wait for the OK button to be pressed and released
    FuriMessageQueue message;
    do {
        // Check for messages and filter for button events
        if(furi_message_queue_get(furi_record_get_message_queue(), &message, FuriWaitForever)) {
            if(message.type == FuriMessageTypeQueue) {
                // Check if the OK button was released
                FuriMessageQueue* button_message = (FuriMessageQueue*)&message.data;
                if(button_message->key == InputKeyOk &&
                   button_message->state == InputTypeShort) {
                    return true;
                }
            }
        }
    } while(true);

    return false;
}
*/