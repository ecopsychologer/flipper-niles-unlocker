#include <furi.h>
#include <api-hal.h>
#include <gui/gui.h>

#define UART_BAUDRATE 38400
#define UART_TX_PIN GPIO_PIN_13
#define UART_RX_PIN GPIO_PIN_14

// Status messages
const char* waiting_for_connection = "Waiting for connection";
const char* connected = "Connected";
const char* transmitting = "Transmitting";
const char* transmitted = "Transmitted";

// Function declarations
void update_display(const char* message);
bool wait_for_ok_button();

void uart_send_data(FuriHalUartId uart_id, const char* data, size_t size) {
    // Ensure UART is initialized with the desired settings
    /**
    FuriHalUartConfig config;
    config.baudrate = UART_BAUDRATE;
    config.tx_pin = UART_TX_PIN;
    config.rx_pin = UART_RX_PIN;
    config.parity = FuriHalUartParityNone;
    config.stop_bits = FuriHalUartStopBits1;
    config.word_length = FuriHalUartWordLength8b;
    furi_hal_uart_init(uart_id, &config);
*/
    FuriHalUartConfig config = {
        .baudrate = UART_BAUDRATE,
        .tx_pin = UART_TX_PIN,
        .rx_pin = UART_RX_PIN,
        .parity = FuriHalUartParityNone,
        .stop_bits = FuriHalUartStopBits1,
        .word_length = FuriHalUartWordLength8b};
    furi_hal_uart_init(uart_id, &config);

    // Send data
    furi_hal_uart_write(uart_id, data, size);
}

int32_t application_main(void* p) {
    UNUSED(p);
    FuriHalUartId uart_id = FuriHalUartIdUART2; // Use the correct UART ID

    const char* trigger = "\x1b"; // Escape character
    const char* response =
        "...# Bam IR Capture Unit Ver 0.35      31Oct05..# Firmware Downloadable...";
    size_t response_length = 74;
    uint8_t rx_byte;

    // UART ID depends on which UART is used; for Flipper Zero, typically FuriHalUartIdUART2
    FuriHalUartId uart_id = FuriHalUartIdUART2;

    while(1) {
        update_display(waiting_for_connection);
        if(furi_hal_uart_read(uart_id, &rx_byte, 1)) {
            update_display(connected);
            if(rx_byte == trigger[0]) {
                update_display(transmitting);
                uart_send_data(uart_id, response, response_length);
                furi_delay_ms(250);
                update_display(transmitted);
                if(wait_for_ok_button()) {
                    // Loop back to the beginning
                    continue;
                }
            }
        }
    }

    return 0;
}

void update_display(const char* message) {
    // Get the GUI context
    Gui* gui = furi_record_open(RECORD_GUI);
    // Draw the message on the screen
    gui_use(gui);
    canvas_clear(gui->canvas);
    canvas_set_font(gui->canvas, FontPrimary);
    canvas_draw_str_aligned(gui->canvas, 64, 32, AlignCenter, AlignCenter, message);
    canvas_draw(gui->canvas);
    gui_release(gui);
    // Note: For more complex UIs, consider using ViewDispatcher and Scenes
}

bool wait_for_ok_button() {
    // Wait for the OK button to be pressed and released
    FuriMessageType message;
    do {
        // Check for messages and filter for button events
        if(furi_message_queue_get(furi_record_get_message_queue(), &message, FuriWaitForever)) {
            if(message.type == FuriMessageTypeButton) {
                // Check if the OK button was released
                FuriMessageButton* button_message = (FuriMessageButton*)&message.data;
                if(button_message->key == InputKeyOk &&
                   button_message->state == InputKeyStateShort) {
                    return true;
                }
            }
        }
    } while(true);

    return false;
}
