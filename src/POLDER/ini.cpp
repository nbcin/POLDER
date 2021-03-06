/*
 * Copyright (C) 2011-2014 Morwenn
 *
 * POLDER is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * POLDER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>
#include <POLDER/ini.h>
#include <POLDER/io.h>
#include <POLDER/string.h>
#include <POLDER/stype.h>


namespace polder
{
namespace ini
{

using namespace string;


// INI-specific elements
namespace
{
    /**
* Return a string without comments
*/
    char* strnorm(const char* str, Dialect dialect)
    {
        char* new_str = new char[strlen(str)];
        int count = -1, i = -1;
        while (str[++i])
        {
            if (str[i] == dialect.lineterminator
                || str[i] == dialect.commentchar)
            {
                count -= 2;
                break;
            }
            new_str[++count] = str[i];
        }
        new_str[count] = '\0';
        return stripped(new_str);
    }
}

Element::Element() = default;
Element::Element(const Element&) = default;
Element::Element(Element&&) = default;
Element::~Element() = default;

Element::Element(const std::string& str):
    _data(str)
{}

Element::operator std::string() const
{
    return _data;
}

Element::operator int() const
{
    return std::stoi(_data);
}

Element::operator long() const
{
    return std::stol(_data);
}

Element::operator long long() const
{
    return std::stoll(_data);
}

Element::operator unsigned() const
{
    return std::stoi(_data);
}

Element::operator unsigned long() const
{
    return std::stoul(_data);
}

Element::operator unsigned long long() const
{
    return std::stoull(_data);
}

Element::operator float() const
{
    return std::stof(_data);
}

Element::operator double() const
{
    return std::stod(_data);
}

Element::operator long double() const
{
    return std::stold(_data);
}

/**
* Return whether the given section exists or not
*/
auto section_exists(const std::string& fname, const std::string& section, Dialect dialect)
    -> bool
{
    // Open the file
    std::ifstream file(fname);
    if (not file)
    {
        throw Error(std::string(__FUNCTION__) + ": " + fname + ": can not open file");
    }

    auto searched = "[" + section + "]";
    std::string line;
    // Read the lines
    while (getline(file, line))
    {
        lstrip(line);
        if (line.substr(0, searched.size()) == searched)
        {
            // The section has been found
            file.close();
            return true;
        }
    }

    // The section has not been found
    file.close();
    return false;
}


/**
* Return whether the given key exists or not
*/
auto key_exists(const std::string& fname, const std::string& section, const std::string& key, Dialect dialect)
    -> bool
{
    // Open the file
    std::ifstream file(fname);
    if (not file)
    {
        throw Error(std::string(__FUNCTION__) + ": " + fname + ": can not open file");
    }

    auto searched = "[" + section + "]";
    bool section_found = false;
    std::string line;
    // Read the lines
    while (getline(file, line))
    {
        strip(line);
        if (not section_found)
        {
            // Search the section
            if (line.substr(0, searched.size()) == searched)
            {
                section_found = true;
            }
        }
        else // If the given section exists
        {
            if (line[0] == '[')
            {
                // We reached another section
                file.close();
                return false;
            }
            else if (line[0] != dialect.commentchar && line[0] != '\0') // We check whether the key is the good one
            {
                int i = -1;
                while (line[++i] != '\0'
                       && line[i] != dialect.lineterminator
                       && line[i] != dialect.delimiter);
                if (line[i] == dialect.delimiter)
                {
                    if (key == stripped(line.substr(0, i)))
                    {
                        // The key has been found
                        file.close();
                        return true;
                    }
                }
                // Else, there is no key, strange...
            }
        }
    }

    // The has not been found
    file.close();
    return false;
}


/**
* Read the string value corresponding to the given key
*/
auto read(const std::string& fname, const std::string& section, const std::string& key, const std::string& default_value, Dialect dialect)
    -> Element
{
    // Open the file
    std::ifstream file(fname);
    if (not file)
    {
        throw Error(std::string(__FUNCTION__) + ": " + fname + ": can not open file");
    }

    auto searched = "[" + section + "]";
    bool section_found = false;
    std::string line;
    // Read the lines
    while (getline(file, line))
    {
        strip(line);
        if (not section_found)
        {
            // Search the section
            if (line.substr(0, searched.size()) == searched)
            {
                section_found = true;
            }
        }
        else // If the given section exists
        {
            if (line[0] == '[')
            {
                // We reached another section
                file.close();
                return default_value;
            }
            else if (line[0] != dialect.commentchar && line[0] != '\0') // We check whether the key is the good one
            {
                int i = -1;
                while (line[++i] != '\0'
                       && line[i] != dialect.lineterminator
                       && line[i] != dialect.delimiter);
                if (line[i] == dialect.delimiter)
                {
                    if (key == stripped(line.substr(0, i)))
                    {
                        // The key has been found
                        int j = i;
                        while (line[++j] != '\0'
                               && line[j] != dialect.lineterminator
                               && line[j] != dialect.commentchar);
                        file.close();
                        return stripped(line.substr(i+1));
                    }
                }
                // Else, there is no key, strange...
            }
        }
    }
    // Finish, an error occurred
    file.close();
    return default_value;
}


/**
* Deletes the given section of an INI file
*/
auto section_delete(const char* fname, const char* section, Dialect dialect)
    -> void
{
    // Open the file
    FILE* f = fopen(fname, "r");
    if (f == nullptr)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": can not open file");
    }

    // Creation of a temporary file
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    FILE* temp = fopen(temp_name, "w");

    // Create the searched word : [section]
    size_t length = strlen(section) + 2;
    char* searched_word = new char[length+1];
    strcpy(searched_word, "[");
    strcat(searched_word, section);
    strcat(searched_word, "]");

    // Read the lines
    bool in_section = false;
    bool section_found = false;
    char* line = nullptr;
    while (io::fgetl(line, f))
    {
        strip(line);
        if (in_section)
        {
            if (line[0] == '[')
            {
                in_section = false;
            }
        }
        else
        {
            if (!strncmp(line, searched_word, length))
            {
                in_section = true;
                section_found = true;
            }
        }

        // Copy the current line in the new file
        if (!in_section)
        {
            fprintf(temp, "%s", line);
        }
    }

    // Finish, check errors
    fclose(f);
    fclose(temp);

    if (!section_found)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": section '" + std::string(section) + "' not found");
    }

    remove(fname);
    rename(temp_name, fname);
}


/**
* Deletes the given key of an INI file
*/
auto key_delete(const char* fname, const char* section, const char* key, Dialect dialect)
    -> void
{
    // Open the file
    FILE* f = fopen(fname, "r");
    if (f == nullptr)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": can not open file");
    }

    // Creation of a temporary file
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    FILE* temp = fopen(temp_name, "w");

    // Create the searched word : [section]
    size_t length = strlen(section) + 2;
    char* searched_word = new char[length+1];
    strcpy(searched_word, "[");
    strcat(searched_word, section);
    strcat(searched_word, "]");

    // Read the lines
    bool in_section = false;
    bool in_key = false;
    bool section_found = false;
    bool key_found = false;
    char* line = nullptr;
    while (io::fgetl(line, f))
    {
        strip(line);
        if (!key_found)
        {
            if (in_section)
            {
                if (line[0] == '[')
                {
                    in_section = false;
                    if (!key_found)
                    {
                        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": key '" + std::string(key) + "' not found");
                    }
                }
                else if (line[0] != dialect.commentchar && line[0] != '\0') // We check whether the key is the good one
                {
                    int i = -1;
                    while (line[++i] != '\0'
                           && line[i] != dialect.lineterminator
                           && line[i] != dialect.delimiter);
                    if (line[i] == dialect.delimiter)
                    {
                        if (!strcmp(key, stripped(substr(line, 0, i-1))))
                        {
                            // The key has been found
                            key_found = true;
                            in_key = true;
                        }
                    }
                    // Else, there is no key, strange...
                }
            }
            else
            {
                if (!strncmp(line, searched_word, length))
                {
                    in_section = true;
                    section_found = true;
                }
            }
        }

        // Copy the current line in the new file
        if (!in_key)
        {
            fprintf(temp, "%s", line);
        }
        else
        {
            in_key = false;
        }
    }

    fclose(f);
    fclose(temp);

    if (!section_found)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": section '" + std::string(section) + "' not found");
    }

    remove(fname);
    rename(temp_name, fname);
}


/**
* Write a string in an INI file
*/
auto write(const char* fname, const char* section, const char* key, const char* value, Dialect dialect)
    -> void
{
    // Open the file
    FILE* f = fopen(fname, "r");
    if (f == nullptr)
    {
        // Create a new file
        f = fopen(fname, "w");
        fprintf(f, "[%s]\n", section);
        fprintf(f, "%s%c%s\n", key, dialect.delimiter, value);
    }

    // Creation of a temporary file
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    FILE* temp = fopen(temp_name, "w");

    // Create the searched word : [section]
    size_t length = strlen(section) + 2;
    char* searched_word = new char[length+1];
    strcpy(searched_word, "[");
    strcat(searched_word, section);
    strcat(searched_word, "]");

    bool section_found = false;
    bool key_found = false;
    bool in_section = false;
    bool write_line = true;
    size_t empty_lines = 0;

    char* line = nullptr;
    while (io::fgetl(line, f))
    {
        strip(line);
        if (!key_found)
        {
            if (section_found == false)
            {
                // Search the section
                if (!strncmp(line, searched_word, length))
                {
                    in_section = true;
                    section_found = true;
                }
            }
            else // If the given section exists
            {
                if (line[0] == dialect.lineterminator)
                {
                    // We "store" the empty line
                    ++empty_lines;
                    write_line = false;
                }
                else if (line[0] == '[')
                {
                    if (in_section)
                    {
                        // We reached the end of the section
                        // The key still has not be found
                        // So we add it
                        fprintf(temp, "%s%c%s\n", key, dialect.delimiter, value);
                        key_found = true;
                    }
                    for (size_t i = 0 ; i < empty_lines ; ++i)
                    {
                        fprintf(temp, "%c", dialect.lineterminator);
                    }
                    empty_lines = 0;
                }
                else
                {
                    // Add the empty lines needed
                    for (size_t i = 0 ; i < empty_lines ; ++i)
                    {
                        fprintf(temp, "%c", dialect.lineterminator);
                    }
                    empty_lines = 0;

                    // We check whether the key is the good one
                    if (line[0] != dialect.commentchar && line[0] != '\0')
                    {
                        strnorm(line, dialect);
                        int i = -1;
                        while (line[++i] != '\0'
                               && line[i] != dialect.lineterminator
                               && line[i] != dialect.delimiter);
                        if (line[i] == dialect.delimiter)
                        {
                            if (!strcmp(key, stripped(substr(line, 0, i-1))))
                            {
                                // The key has been found
                                fprintf(temp, "%s%c%s\n", key, dialect.delimiter, value);
                                key_found = true;
                                write_line = false;
                            }
                        }
                    }
                }
            }
        }

        // Copy the current line in the new file
        if (write_line)
        {
            fprintf(temp, "%s", line);
        }
        else
        {
            write_line = true;
        }
    }

    if (!section_found)
    {
        fprintf(temp, "\n");
        fprintf(temp, "%s\n", searched_word);
        fprintf(temp, "%s%c%s\n", key, dialect.delimiter, value);
    }
    else if (!key_found)
    {
        fprintf(temp, "%s%c%s\n", key, dialect.delimiter, value);
    }

    // Finish
    fclose(f);
    fclose(temp);
    remove(fname);
    rename(temp_name, fname);
}


/**
* Write a real in an INI file
*/
auto write(const char* fname, const char* section, const char* key, double value, Dialect dialect)
    -> void
{
    // Open the file
    FILE* f = fopen(fname, "r");
    if (f == nullptr)
    {
        // Create a new file
        f = fopen(fname, "w");
        fprintf(f, "[%s]\n", section);
        fprintf(f, "%s%c%f\n", key, dialect.delimiter, value);
    }

    // Creation of a temporary file
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    FILE* temp = fopen(temp_name, "w");

    // Create the searched word : [section]
    size_t length = strlen(section) + 2;
    char* searched_word = new char[length+1];
    strcpy(searched_word, "[");
    strcat(searched_word, section);
    strcat(searched_word, "]");

    bool section_found = false;
    bool key_found = false;
    bool in_section = false;
    bool write_line = true;
    size_t empty_lines = 0;

    char* line = nullptr;
    while (io::fgetl(line, f))
    {
        strip(line);
        if (!key_found)
        {
            if (section_found == false)
            {
                // Search the section
                if (!strncmp(line, searched_word, length))
                {
                    in_section = true;
                    section_found = true;
                }
            }
            else // If the given section exists
            {
                if (line[0] == dialect.lineterminator)
                {
                    // We "store" the empty line
                    ++empty_lines;
                    write_line = false;
                }
                else if (line[0] == '[')
                {
                    if (in_section)
                    {
                        // We reached the end of the section
                        // The key still has not be found
                        // So we add it
                        fprintf(temp, "%s%c%f\n", key, dialect.delimiter, value);
                        key_found = true;
                    }
                    for (size_t i = 0 ; i < empty_lines ; ++i)
                    {
                        fprintf(temp, "%c", dialect.lineterminator);
                    }
                    empty_lines = 0;
                }
                else
                {
                    // Add the empty lines needed
                    for (size_t i = 0 ; i < empty_lines ; ++i)
                    {
                        fprintf(temp, "%c", dialect.lineterminator);
                    }
                    empty_lines = 0;


                    // We check whether the key is the good one
                    if (line[0] != dialect.commentchar && line[0] != '\0')
                    {
                        strnorm(line, dialect);
                        int i = -1;
                        while (line[++i] != '\0'
                               && line[i] != dialect.lineterminator
                               && line[i] != dialect.delimiter);
                        if (line[i] == dialect.delimiter)
                        {
                            if (!strcmp(key, stripped(substr(line, 0, i-1))))
                            {
                                // The key has been found
                                fprintf(temp, "%s%c%f\n", key, dialect.delimiter, value);
                                key_found = true;
                                write_line = false;
                            }
                        }
                    }
                }
            }
        }

        // Copy the current line in the new file
        if (write_line)
        {
            fprintf(temp, "%s", line);
        }
        else
        {
            write_line = true;
        }
    }

    if (!section_found)
    {
        fprintf(temp, "\n");
        fprintf(temp, "%s\n", searched_word);
        fprintf(temp, "%s%c%f\n", key, dialect.delimiter, value);
    }
    else if (!key_found)
    {
        fprintf(temp, "%s%c%f\n", key, dialect.delimiter, value);
    }

    // Finish
    fclose(f);
    fclose(temp);
    remove(fname);
    rename(temp_name, fname);
}

/**
* Renames the given section of an INI file
*/
auto section_rename(const char* fname, const char* section, const char* new_section, Dialect dialect)
    -> void
{
    // Open the file
    FILE* f = fopen(fname, "r");
    if (f == nullptr)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": can not open file");
    }

    // Creation of a temporary file
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    FILE* temp = fopen(temp_name, "w");

    // Create the searched word : [section]
    size_t length = strlen(section) + 2;
    char* searched_word = new char[length+1];
    strcpy(searched_word, "[");
    strcat(searched_word, section);
    strcat(searched_word, "]");

    // Create the searched word : [new_section]
    size_t new_length = strlen(new_section) + 2;
    char* new_searched_word = new char[new_length+1];
    strcpy(new_searched_word, "[");
    strcat(new_searched_word, new_section);
    strcat(new_searched_word, "]");

    bool section_found = false;
    bool write_line = true;
    char* line = nullptr;
    while (io::fgetl(line, f))
    {
        lstrip(line);
        if (!strncmp(line, searched_word, length))
        {
            fprintf(temp, "%s%c", new_searched_word, dialect.lineterminator);
            write_line = false;
            section_found = true;
        }
        else if (!strncmp(line, new_searched_word, new_length))
        {
            fclose(f);
            fclose(temp);
            throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": section '" + std::string(new_section) + "' already exists");
        }

        // Copy the current line in the new file
        if (write_line)
        {
            fprintf(temp, "%s", line);
        }
        else
        {
            write_line = true;
        }
    }

    // Finish, check errors
    fclose(f);
    fclose(temp);
    remove(fname);
    rename(temp_name, fname);

    if (!section_found)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": section '" + std::string(section) + "' not found");
    }
}

/**
* Renames the given key of an INI file
*/
auto key_rename(const char* fname, const char* section, const char* key, const char* new_key, Dialect dialect)
    -> void
{
    // Open the file
    FILE* f = fopen(fname, "r");
    if (f == nullptr)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": can not open file");
    }

    // Creation of a temporary file
    char temp_name[L_tmpnam];
    tmpnam(temp_name);
    FILE* temp = fopen(temp_name, "w");

    // Create the searched word : [section]
    size_t length = strlen(section) + 2;
    char* searched_word = new char[length+1];
    strcpy(searched_word, "[");
    strcat(searched_word, section);
    strcat(searched_word, "]");

    bool section_found = false;
    bool key_found = false;
    bool in_section = false;
    bool write_line = true;
    char* line = nullptr;
    while (io::fgetl(line, f))
    {
        if (!key_found)
        {
            strip(line);
            if (!section_found)
            {
                if (!strncmp(line, searched_word, length))
                {
                    section_found = true;
                    in_section = true;
                }
            }
            else if (in_section)
            {
                if (line[0] == '[')
                {
                    throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": key '" + std::string(key) + "' not found");
                }
                // We check whether the key is the good one
                else if (line[0] != dialect.commentchar && line[0] != '\0')
                {
                    strnorm(line, dialect);
                    int i = -1;
                    while (line[++i] != '\0'
                           && line[i] != dialect.lineterminator
                           && line[i] != dialect.delimiter);
                    if (line[i] == dialect.delimiter)
                    {
                        if (!strcmp(key, stripped(substr(line, 0, i-1))))
                        {
                            // The key has been found
                            key_found = true;
                            write_line = false;
                            int j = i;
                            while (line[++j] != '\0'
                                   && line[j] != dialect.lineterminator
                                   && line[j] != dialect.commentchar);
                            fprintf(temp, "%s%c%s\n", new_key, dialect.delimiter, substr(line, i+1, j-1));
                        }
                        else if (!strcmp(new_key, stripped(substr(line, 0, i-1))))
                        {
                            throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": key '" + std::string(new_key) + "' already exists");
                        }
                    }
                }
            }
        }

        // Copy the current line in the new file
        if (write_line)
        {
            fprintf(temp, "%s", line);
        }
        else
        {
            write_line = true;
        }
    }

    // Finish, check errors
    fclose(f);
    fclose(temp);

    if (!section_found)
    {
        throw Error(std::string(__FUNCTION__) + ": " + std::string(fname) + ": section '" + std::string(section) + "' not found");
    }

    remove(fname);
    rename(temp_name, fname);
}


/**
* Exceptions handling
*/

// Create a new exception
Error::Error()
{
    std::ostringstream oss;
    oss << "polder::ini::Error: undocumented error.";
    msg = oss.str();
}

// Create a new exception
Error::Error(const std::string& arg)
{
    std::ostringstream oss;
    oss << arg;
    msg = oss.str();
}

// Destructor, does nothing
Error::~Error() noexcept {}

// Returns what the error is
const char* Error::what() const noexcept
{
    return msg.c_str();
}


} // namespace ini
} // namespace polder
