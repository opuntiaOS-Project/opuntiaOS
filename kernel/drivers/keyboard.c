#include <driverManager.h>
#include <keyboard.h>
#include <display.h>
#include <idt.h>

void keyboard_handler() {
    u_int8 scancode = port_byte_in(0x60);
    print_letter(scancode);
}

void register_keyboard_driver() { // all drivers start with main
    register_driver(start_keyboard, stop_keyboard);
}

char start_keyboard() {
    printf("Started keyboard driver\n");
    setup_irq_handler(IRQ1, keyboard_handler);
    return TRUE;
}

void stop_keyboard() {
    printf("Stopped keyboard driver\n");
}

void print(char *l){
    print_string(l, WHITE_ON_BLACK, -1, -1);
}

void print_letter(u_int8 scancode) {
    switch (scancode) {
        case 0x0:
            print("ERROR");
            break;
        case 0x1:
            print("ESC");
            break;
        case 0x2:
            print("1");
            break;
        case 0x3:
            print("2");
            break;
        case 0x4:
            print("3");
            break;
        case 0x5:
            print("4");
            break;
        case 0x6:
            print("5");
            break;
        case 0x7:
            print("6");
            break;
        case 0x8:
            print("7");
            break;
        case 0x9:
            print("8");
            break;
        case 0x0A:
            print("9");
            break;
        case 0x0B:
            print("0");
            break;
        case 0x0C:
            print("-");
            break;
        case 0x0D:
            print("+");
            break;
        case 0x0E:
            print("Backspace");
            break;
        case 0x0F:
            print("Tab");
            break;
        case 0x10:
            print("Q");
            break;
        case 0x11:
            print("W");
            break;
        case 0x12:
            print("E");
            break;
        case 0x13:
            print("R");
            break;
        case 0x14:
            print("T");
            break;
        case 0x15:
            print("Y");
            break;
        case 0x16:
            print("U");
            break;
        case 0x17:
            print("I");
            break;
        case 0x18:
            print("O");
            break;
        case 0x19:
            print("P");
            break;
		case 0x1A:
			print("[");
			break;
		case 0x1B:
			print("]");
			break;
		case 0x1C:
			print("ENTER");
			break;
		case 0x1D:
			print("LCtrl");
			break;
		case 0x1E:
			print("A");
			break;
		case 0x1F:
			print("S");
			break;
        case 0x20:
            print("D");
            break;
        case 0x21:
            print("F");
            break;
        case 0x22:
            print("G");
            break;
        case 0x23:
            print("H");
            break;
        case 0x24:
            print("J");
            break;
        case 0x25:
            print("K");
            break;
        case 0x26:
            print("L");
            break;
        case 0x27:
            print(";");
            break;
        case 0x28:
            print("'");
            break;
        case 0x29:
            print("`");
            break;
		case 0x2A:
			print("LShift");
			break;
		case 0x2B:
			print("\\");
			break;
		case 0x2C:
			print("Z");
			break;
		case 0x2D:
			print("X");
			break;
		case 0x2E:
			print("C");
			break;
		case 0x2F:
			print("V");
			break;
        case 0x30:
            print("B");
            break;
        case 0x31:
            print("N");
            break;
        case 0x32:
            print("M");
            break;
        case 0x33:
            print(",");
            break;
        case 0x34:
            print(".");
            break;
        case 0x35:
            print("/");
            break;
        case 0x36:
            print("Rshift");
            break;
        case 0x37:
            print("Keypad *");
            break;
        case 0x38:
            print("LAlt");
            break;
        case 0x39:
            print("Spc");
            break;
        default:
            if (scancode <= 0x7f) {
                print("Unknown key down");
            } else if (scancode <= 0x39 + 0x80) {
                print("key up ");
                print_letter(scancode - 0x80);
            } else print("Unknown key up");
            break;
    }
}
