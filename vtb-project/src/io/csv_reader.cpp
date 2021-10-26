#include "csv_reader.hpp"

#include <algorithm>

std::vector<std::string> split(std::string& s, const std::string& delim=" ")
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    while((pos = s.find(delim)) != std::string::npos) {
        tokens.push_back(s.substr(0, pos));
        s.erase(0, pos + delim.length());
    }
    tokens.push_back(s); //last word
    return tokens;
}

CSVRow::CSVRow(std::ifstream& file, bool hasHeader) {
            if(hasHeader) {
                std::string line;
                std::getline(file, line);
                m_columns = split(line, ",");
            }
}

CSVRow::CSVRow() {}

std::string CSVRow::operator[](std::size_t index) const
        {
            return std::string(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
        }

std::string CSVRow::operator[](std::string column) const 
        {
            auto it = std::find(m_columns.begin(), m_columns.end(), column);
            if (it != m_columns.end()) {
                int index = std::distance(m_columns.begin(), it);
                return std::string(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
            }
            else {
                throw "Element Not Found";
                return std::string();
            }
        }

std::size_t CSVRow::size() const
        {
            return m_data.size() - 1;
        }

void CSVRow::readNextRow(std::istream& str)
        {
            std::getline(str, m_line);

            m_data.clear();
            m_data.emplace_back(-1);
            std::string::size_type pos = 0;
            while((pos = m_line.find(',', pos)) != std::string::npos)
            {
                m_data.emplace_back(pos);
                ++pos;
            }
            // This checks for a trailing comma with no data after it.
            pos   = m_line.size();
            m_data.emplace_back(pos);
        }

void CSVRow::readNextRow() {
    
}

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}   
