#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>

/* Define for Header */
#define __GSL_INCLUDED__

#include "MatD.h"

#include <iostream>
#include <libxml++/libxml++.h>

namespace obvious {

MatD::MatD(const unsigned int cols, const unsigned int rows)
    : AbstractMat(cols, rows)
{
    m_data = gsl_matrix_alloc(rows, cols);
}

MatD::MatD(const MatD& mat)
    : AbstractMat(mat.m_cols, mat.m_rows)
{
    gsl_matrix_memcpy(m_data, mat.m_data);
}

MatD::MatD(const xmlpp::Node* node)
    : AbstractMat(0, 0)
{
    /* Check if the XML tag has the name mat */
    if (node->get_name() != "mat")
    {
        throw "Invaild xml node for matrix initialization!";
        return;
    }

    /* Check if node is from type xmlpp::Element. If not throw an exeption and return */
    const xmlpp::Element* root = dynamic_cast<const xmlpp::Element*>(node);

    if (!root)
    {
        throw "Invaild xml node for matrix initialization!";
        return;
    }

    /* Read attributes cols and rows to know the dimension of the MatD */
    std::stringstream stream(root->get_attribute_value("cols"));
    unsigned int rows, cols;

    stream >> rows;
    stream.clear();
    stream.str(root->get_attribute_value("rows"));
    stream >> cols;

    /* If any of both has a null as value return */
    if (!rows || !cols)
        return;

    /* Allocate matrix */
    m_data = gsl_matrix_alloc(rows, cols);
    m_rows = rows;
    m_cols = cols;

    /* Read values for the matrix */
    const xmlpp::Node::NodeList nodes = root->get_children();
    xmlpp::Node::NodeList::const_iterator it = nodes.begin();

    for (unsigned int row = 0; it != nodes.end() && row < rows; ++it, ++row)
    {
        xmlpp::Element* elm = dynamic_cast<xmlpp::Element*>(*it);

        if (!elm && elm->get_name() != "row")
            continue;

        stream.clear();
        stream.str(elm->get_child_text()->get_content());

        for (unsigned int col = 0; col < cols; col++)
        {
            double value;

            stream >> value;
            gsl_matrix_set(m_data, row, col, value);
        }
    }
}

MatD::~MatD(void)
{
    /* Check if m_data has to be deleted */
    if (this->haveToFreeData())
        gsl_matrix_free(m_data);
}

void MatD::createXml(xmlpp::Node* node) const
{
    /* Create tag mat with attributes cols and rows */
    xmlpp::Element* root = node->add_child("mat");
    std::stringstream stream;

    stream << m_cols;
    root->set_attribute("cols", stream.str());

    stream.str(std::string());
    stream << m_rows;
    root->set_attribute("rows", stream.str());

    /* Write the data from matrix m_data to xml nodes */
    for (unsigned int row = 0; row < m_rows; row++)
    {
        xmlpp::Element* elm = root->add_child("row");
        stream.str(std::string());

        for (unsigned int col = 0; col < m_cols; col++)
            stream << gsl_matrix_get(m_data, row, col) << " ";

        elm->add_child_text(stream.str());
    }
}

double& MatD::at(const unsigned int col, const unsigned int row)
{
    return *gsl_matrix_ptr(m_data, row, col);
}

double MatD::at(const unsigned int col, const unsigned int row) const
{
    return gsl_matrix_get(m_data, row, col);
}

MatD& MatD::operator=(MatD& mat)
{
    /* Delete m_data before take a ref to another Mat */
    if (m_data)
        gsl_matrix_free(m_data);

    AbstractMat<double>::operator=(mat);

    return *this;
}

MatD MatD::operator*(const MatD& mat)
{
    MatD matN(m_data->size1, mat.m_data->size2);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, m_data, mat.m_data, 0.0, matN.m_data);
    return matN;
}

}


std::ostream& operator<<(std::ostream& os, const obvious::MatD& mat)
{
    os << "Matrix (" << mat.cols() << "," << mat.rows() << ")" << std::endl;
    os << "----------------------------" << std::endl;

    for (unsigned int row = 0; row < mat.rows(); row++)
    {
        os << row << ": ";

        for (unsigned int col = 0; col < mat.cols(); col++)
        {
            os << mat.at(col, row) << " ";
        }

        os << std::endl;
    }

    return os;
}