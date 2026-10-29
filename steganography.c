#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================
   IMAGE STEGANOGRAPHY PROJECT IN C
   Hides text inside a 24-bit BMP image using LSB
   ========================================================= */


/* ---------------------------------------------------------
   Function: encode_bit
   Stores one message bit in the LSB of an image byte
   --------------------------------------------------------- */
void encode_bit(unsigned char *image_byte, unsigned char bit)
{
    *image_byte = (*image_byte & 0xFE) | bit;
}


/* ---------------------------------------------------------
   Function: encode_char
   Stores one character using 8 image bytes
   --------------------------------------------------------- */
void encode_char(unsigned char *image_data, unsigned char ch)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        unsigned char bit;

        bit = (ch >> (7 - i)) & 1;

        encode_bit(&image_data[i], bit);
    }
}


/* ---------------------------------------------------------
   Function: decode_char
   Extracts one character from 8 image bytes
   --------------------------------------------------------- */
unsigned char decode_char(unsigned char *image_data)
{
    unsigned char ch = 0;

    int i;

    for (i = 0; i < 8; i++)
    {
        unsigned char bit;

        bit = image_data[i] & 1;

        ch = (ch << 1) | bit;
    }

    return ch;
}


/* ---------------------------------------------------------
   Function: encode_message
   Hides a text message inside a BMP image
   --------------------------------------------------------- */
int encode_message(
    const char *source_image,
    const char *output_image,
    const char *message)
{
    FILE *source;
    FILE *output;

    unsigned char *image_data;

    long file_size;
    long position;

    const int header_size = 54;

    /* Open source image */
    source = fopen(source_image, "rb");

    if (source == NULL)
    {
        printf("\nError: Cannot open source image.\n");
        return 0;
    }


    /* Find image size */
    fseek(source, 0, SEEK_END);

    file_size = ftell(source);

    rewind(source);


    /* Check image size */
    if (file_size <= header_size)
    {
        printf("\nError: Invalid BMP image.\n");

        fclose(source);

        return 0;
    }


    /* Allocate memory */
    image_data = (unsigned char *)malloc(file_size);

    if (image_data == NULL)
    {
        printf("\nError: Memory allocation failed.\n");

        fclose(source);

        return 0;
    }


    /* Read image */
    if (fread(image_data, 1, file_size, source)
        != (size_t)file_size)
    {
        printf("\nError: Cannot read image.\n");

        free(image_data);

        fclose(source);

        return 0;
    }


    fclose(source);


    /* Check BMP signature */
    if (image_data[0] != 'B' ||
        image_data[1] != 'M')
    {
        printf("\nError: File is not a BMP image.\n");

        free(image_data);

        return 0;
    }


    /* Calculate required space */
    long required_bytes;

    required_bytes =
        (strlen(message) + 1) * 8;


    /* Calculate available space */
    long available_bytes;

    available_bytes =
        file_size - header_size;


    /* Check capacity */
    if (required_bytes > available_bytes)
    {
        printf("\nError: Message is too large.\n");

        printf("Available bytes: %ld\n",
               available_bytes);

        printf("Required bytes: %ld\n",
               required_bytes);

        free(image_data);

        return 0;
    }


    /* Start after BMP header */
    position = header_size;


    /* Encode message */
    for (size_t i = 0;
         i <= strlen(message);
         i++)
    {
        encode_char(
            &image_data[position],
            (unsigned char)message[i]
        );

        position += 8;
    }


    /* Create output image */
    output = fopen(output_image, "wb");

    if (output == NULL)
    {
        printf("\nError: Cannot create output image.\n");

        free(image_data);

        return 0;
    }


    /* Write modified image */
    if (fwrite(image_data, 1, file_size, output)
        != (size_t)file_size)
    {
        printf("\nError: Cannot write output image.\n");

        fclose(output);

        free(image_data);

        return 0;
    }


    fclose(output);

    free(image_data);


    printf("\n====================================\n");
    printf("Message encoded successfully!\n");
    printf("Output image: %s\n",
           output_image);
    printf("====================================\n");

    return 1;
}


/* ---------------------------------------------------------
   Function: decode_message
   Extracts hidden text from BMP image
   --------------------------------------------------------- */
int decode_message(const char *stego_image)
{
    FILE *file;

    unsigned char *image_data;

    long file_size;
    long position;

    const int header_size = 54;


    /* Open stego image */
    file = fopen(stego_image, "rb");

    if (file == NULL)
    {
        printf("\nError: Cannot open stego image.\n");

        return 0;
    }


    /* Find file size */
    fseek(file, 0, SEEK_END);

    file_size = ftell(file);

    rewind(file);


    /* Check size */
    if (file_size <= header_size)
    {
        printf("\nError: Invalid BMP image.\n");

        fclose(file);

        return 0;
    }


    /* Allocate memory */
    image_data =
        (unsigned char *)malloc(file_size);

    if (image_data == NULL)
    {
        printf("\nError: Memory allocation failed.\n");

        fclose(file);

        return 0;
    }


    /* Read image */
    if (fread(image_data, 1, file_size, file)
        != (size_t)file_size)
    {
        printf("\nError: Cannot read image.\n");

        free(image_data);

        fclose(file);

        return 0;
    }


    fclose(file);


    /* Check BMP */
    if (image_data[0] != 'B' ||
        image_data[1] != 'M')
    {
        printf("\nError: File is not a BMP image.\n");

        free(image_data);

        return 0;
    }


    /* Start after BMP header */
    position = header_size;


    printf("\n====================================\n");
    printf("Hidden Message:\n");
    printf("====================================\n");


    /* Decode message */
    while (position + 8 <= file_size)
    {
        unsigned char ch;

        ch = decode_char(
            &image_data[position]
        );


        /* NULL means message is finished */
        if (ch == '\0')
        {
            break;
        }


        printf("%c", ch);

        position += 8;
    }


    printf("\n====================================\n");


    free(image_data);

    return 1;
}


/* ---------------------------------------------------------
   MAIN FUNCTION
   --------------------------------------------------------- */
int main()
{
    int choice;

    char source_image[200];

    char output_image[200];

    char message[1000];


    printf("\n");
    printf("============================================\n");
    printf("        IMAGE STEGANOGRAPHY IN C\n");
    printf("============================================\n");


    while (1)
    {
        printf("\n");

        printf("1. Encode Message\n");

        printf("2. Decode Message\n");

        printf("3. Exit\n");


        printf("\nEnter your choice: ");

        scanf("%d", &choice);

        /* Remove newline */
        getchar();


        /* =========================================
           ENCODE
           ========================================= */
        if (choice == 1)
        {
            printf("\nEnter source BMP image: ");

            fgets(
                source_image,
                sizeof(source_image),
                stdin
            );


            source_image[
                strcspn(source_image, "\n")
            ] = '\0';


            printf("Enter output BMP image: ");

            fgets(
                output_image,
                sizeof(output_image),
                stdin
            );


            output_image[
                strcspn(output_image, "\n")
            ] = '\0';


            printf("Enter secret message: ");

            fgets(
                message,
                sizeof(message),
                stdin
            );


            message[
                strcspn(message, "\n")
            ] = '\0';


            encode_message(
                source_image,
                output_image,
                message
            );
        }


        /* =========================================
           DECODE
           ========================================= */
        else if (choice == 2)
        {
            printf("\nEnter stego BMP image: ");


            fgets(
                source_image,
                sizeof(source_image),
                stdin
            );


            source_image[
                strcspn(source_image, "\n")
            ] = '\0';


            decode_message(source_image);
        }


        /* =========================================
           EXIT
           ========================================= */
        else if (choice == 3)
        {
            printf("\nThank you for using "
                   "Steganography!\n");

            break;
        }


        /* =========================================
           INVALID CHOICE
           ========================================= */
        else
        {
            printf("\nInvalid choice!\n");

            printf("Please enter 1, 2 or 3.\n");
        }
    }


    return 0;
}