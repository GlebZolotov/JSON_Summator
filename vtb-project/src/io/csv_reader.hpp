#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include <string>


class CSVRow
{
    public:
        CSVRow(std::ifstream& file, bool hasHeader=true);

        CSVRow();

        std::vector<int> getData() const {return m_data;}

        std::string operator[](std::size_t index) const;
        std::string operator[](std::string column) const;

        std::size_t size() const;

        void readNextRow(std::istream& str);
        void readNextRow();
    private:
        std::vector<std::string> m_columns;
        std::string         m_line;
        std::vector<int>    m_data;
        std::ifstream       m_file;
};

std::istream& operator>>(std::istream& str, CSVRow& data);