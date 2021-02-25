#include <stdio.h>
#include <unistd.h>
#include "parson.h"

static void serialization_example(void);
static void parse_example(void);

void main(void)
{
    serialization_example();
    parse_example();
    return;
}

static void serialization_example(void)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "name", "John Smith");
    json_object_set_number(root_object, "age", 25);
    json_object_set_value(root_object, "read_messages", json_parse_string("[\"001\",\"002\"]"));
    json_object_dotset_string(root_object, "address.city", "Cupertino");
    json_object_dotset_value(root_object, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
    serialized_string = json_serialize_to_string_pretty(root_value);
    puts(serialized_string);

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}


static char * json_str = "{\"age\":16, \"name\":\"John\", \"grade\":{\"Maths\":\"A\", \"English\":\"B\"}}";
static void parse_example(void)
{
    JSON_Value *root_value;
    JSON_Object *root_object;

    root_value = json_parse_string(json_str);
    root_object = json_value_get_object(root_value);

    const char *str_name = json_object_get_string(root_object, "name");
    if(str_name) {
        printf("name= %s\n", str_name);
    }

    int age = json_object_get_number(root_object, "age");
    printf("age= %d\n", age);

    const char *str_maths_grade = json_object_dotget_string(root_object, "grade.Maths");
    if(str_maths_grade) {
        printf("str_maths_grade= %s\n", str_maths_grade);
    }
    const char *str_english_grade = json_object_dotget_string(root_object, "grade.English");
    if(str_english_grade) {
        printf("str_maths_grade= %s\n", str_english_grade);
    }

}