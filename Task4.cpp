#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

// Structure to store product information
struct Product {
    string name;
    string price;
    string rating;
    string url;
    string description;
    
    Product() : rating("0.0") {}
    
    Product(const string& n, const string& p, const string& r, const string& u, const string& d = "") 
        : name(n), price(p), rating(r), url(u), description(d) {}
};

class EcommerceScraper {
private:
    vector<string> sampleNames = {
        "iPhone 14 Pro Max 128GB Space Black",
        "Samsung Galaxy S23 Ultra 256GB Phantom Black",
        "MacBook Air M2 13-inch Silver",
        "Sony WH-1000XM4 Wireless Headphones",
        "Dell XPS 13 Laptop Intel i7 16GB RAM",
        "iPad Air 5th Generation 64GB WiFi",
        "Nintendo Switch OLED White",
        "Apple AirPods Pro 2nd Generation",
        "Amazon Kindle Paperwhite 11th Gen",
        "Echo Dot 5th Gen with Alexa",
        "Samsung 55-inch 4K Smart TV",
        "HP LaserJet Pro Printer",
        "Logitech MX Master 3S Wireless Mouse",
        "JBL Flip 6 Bluetooth Speaker",
        "Canon EOS R6 Mark II Camera Body",
        "Microsoft Surface Pro 9 Tablet",
        "Google Nest Hub Max Smart Display",
        "Fitbit Charge 5 Fitness Tracker",
        "Bose QuietComfort 45 Headphones",
        "LG UltraWide 34-inch Monitor"
    };
    
    vector<string> samplePrices = {
        "$1099.00", "$1199.99", "$1199.00", "$349.99", "$999.99",
        "$599.00", "$349.99", "$249.00", "$139.99", "$49.99",
        "$799.99", "$229.99", "$99.99", "$129.99", "$2499.00",
        "$1099.99", "$229.99", "$149.95", "$329.00", "$499.99"
    };

public:
    EcommerceScraper() {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    // Clean and extract text from HTML tags
    string cleanText(const string& text) {
        string cleaned = text;
        
        // Remove HTML tags
        regex htmlTags("<[^>]*>");
        cleaned = regex_replace(cleaned, htmlTags, "");
        
        // Decode common HTML entities
        regex nbsp("&nbsp;");
        cleaned = regex_replace(cleaned, nbsp, " ");
        regex amp("&amp;");
        cleaned = regex_replace(cleaned, amp, "&");
        regex lt("&lt;");
        cleaned = regex_replace(cleaned, lt, "<");
        regex gt("&gt;");
        cleaned = regex_replace(cleaned, gt, ">");
        regex quot("&quot;");
        cleaned = regex_replace(cleaned, quot, "\"");
        
        // Remove extra whitespace
        regex extraSpaces("\\s+");
        cleaned = regex_replace(cleaned, extraSpaces, " ");
        
        // Trim leading and trailing spaces
        size_t start = cleaned.find_first_not_of(" \t\n\r");
        if (start == string::npos) return "";
        size_t end = cleaned.find_last_not_of(" \t\n\r");
        cleaned = cleaned.substr(start, end - start + 1);
        
        return cleaned;
    }

    // Escape text for CSV format
    string escapeCSV(const string& text) {
        string escaped = text;
        
        // If text contains comma, quote, or newline, wrap in quotes
        if (escaped.find(',') != string::npos || 
            escaped.find('"') != string::npos || 
            escaped.find('\n') != string::npos) {
            
            // Replace internal quotes with double quotes
            regex quotes("\"");
            escaped = regex_replace(escaped, quotes, "\"\"");
            escaped = "\"" + escaped + "\"";
        }
        
        return escaped;
    }

    // Extract products from HTML content
    vector<Product> extractProducts(const string& html) {
        vector<Product> products;
        
        // Multiple patterns for different e-commerce structures
        vector<regex> productContainerPatterns = {
            // Amazon-like patterns
            regex("<div[^>]*data-component-type=\"s-search-result\"[^>]*>(.*?)</div>\\s*</div>\\s*</div>", 
                  regex_constants::icase),
            
            // Generic product containers
            regex("<div[^>]*class=\"[^\"]*product[^\"]*item[^\"]*\"[^>]*>(.*?)</div>", 
                  regex_constants::icase),
            regex("<div[^>]*class=\"[^\"]*product[^\"]*card[^\"]*\"[^>]*>(.*?)</div>", 
                  regex_constants::icase),
            regex("<article[^>]*class=\"[^\"]*product[^\"]*\"[^>]*>(.*?)</article>", 
                  regex_constants::icase),
            
            // List item patterns
            regex("<li[^>]*class=\"[^\"]*product[^\"]*\"[^>]*>(.*?)</li>", 
                  regex_constants::icase),
            
            // Generic item patterns
            regex("<div[^>]*class=\"[^\"]*item[^\"]*\"[^>]*>(.*?)</div>", 
                  regex_constants::icase)
        };
        
        // Name extraction patterns
        vector<regex> namePatterns = {
            // Heading patterns
            regex("<h[1-6][^>]*class=\"[^\"]*title[^\"]*\"[^>]*>(.*?)</h[1-6]>", regex_constants::icase),
            regex("<h[1-6][^>]*class=\"[^\"]*name[^\"]*\"[^>]*>(.*?)</h[1-6]>", regex_constants::icase),
            regex("<h[1-6][^>]*class=\"[^\"]*product[^\"]*title[^\"]*\"[^>]*>(.*?)</h[1-6]>", regex_constants::icase),
            
            // Link patterns
            regex("<a[^>]*class=\"[^\"]*title[^\"]*\"[^>]*>(.*?)</a>", regex_constants::icase),
            regex("<a[^>]*class=\"[^\"]*name[^\"]*\"[^>]*>(.*?)</a>", regex_constants::icase),
            regex("<a[^>]*class=\"[^\"]*product[^\"]*link[^\"]*\"[^>]*>(.*?)</a>", regex_constants::icase),
            
            // Span and div patterns
            regex("<span[^>]*class=\"[^\"]*title[^\"]*\"[^>]*>(.*?)</span>", regex_constants::icase),
            regex("<div[^>]*class=\"[^\"]*name[^\"]*\"[^>]*>(.*?)</div>", regex_constants::icase),
            regex("<span[^>]*class=\"[^\"]*name[^\"]*\"[^>]*>(.*?)</span>", regex_constants::icase)
        };
        
        // Price extraction patterns
        vector<regex> pricePatterns = {
            // Class-based patterns
            regex("<span[^>]*class=\"[^\"]*price[^\"]*\"[^>]*>(.*?)</span>", regex_constants::icase),
            regex("<div[^>]*class=\"[^\"]*price[^\"]*\"[^>]*>(.*?)</div>", regex_constants::icase),
            regex("<p[^>]*class=\"[^\"]*price[^\"]*\"[^>]*>(.*?)</p>", regex_constants::icase),
            
            // Currency patterns
            regex("\\$\\s*[\\d,]+\\.?\\d*", regex_constants::icase),
            regex("₹\\s*[\\d,]+\\.?\\d*", regex_constants::icase),
            regex("€\\s*[\\d,]+\\.?\\d*", regex_constants::icase),
            regex("£\\s*[\\d,]+\\.?\\d*", regex_constants::icase),
            regex("USD\\s*[\\d,]+\\.?\\d*", regex_constants::icase),
            regex("INR\\s*[\\d,]+\\.?\\d*", regex_constants::icase),
            
            // Generic price patterns
            regex("Price:\\s*([\\d,]+\\.?\\d*)", regex_constants::icase),
            regex("Cost:\\s*([\\d,]+\\.?\\d*)", regex_constants::icase)
        };
        
        // Rating extraction patterns
        vector<regex> ratingPatterns = {
            regex("<span[^>]*class=\"[^\"]*rating[^\"]*\"[^>]*>([0-9.]+)</span>", regex_constants::icase),
            regex("<div[^>]*class=\"[^\"]*star[^\"]*\"[^>]*>([0-9.]+)</div>", regex_constants::icase),
            regex("<span[^>]*class=\"[^\"]*star[^\"]*\"[^>]*>([0-9.]+)</span>", regex_constants::icase),
            regex("([0-9.]+)\\s*out\\s*of\\s*[0-9]+", regex_constants::icase),
            regex("([0-9.]+)\\s*/\\s*[0-9]+", regex_constants::icase),
            regex("Rating:\\s*([0-9.]+)", regex_constants::icase),
            regex("★\\s*([0-9.]+)", regex_constants::icase)
        };
        
        cout << "Analyzing HTML content..." << endl;
        
        // Try different container patterns
        for (size_t patternIndex = 0; patternIndex < productContainerPatterns.size(); patternIndex++) {
            const auto& containerPattern = productContainerPatterns[patternIndex];
            
            sregex_iterator iter(html.begin(), html.end(), containerPattern);
            sregex_iterator end;
            
            int foundWithThisPattern = 0;
            
            for (; iter != end && products.size() < 100; ++iter) {
                string productHtml = iter->str(1);
                Product product;
                
                // Extract name
                for (const auto& namePattern : namePatterns) {
                    smatch match;
                    if (regex_search(productHtml, match, namePattern)) {
                        string candidateName = cleanText(match[1]);
                        if (candidateName.length() > 5 && candidateName.length() < 200) {
                            product.name = candidateName;
                            break;
                        }
                    }
                }
                
                // Extract price
                for (const auto& pricePattern : pricePatterns) {
                    smatch match;
                    if (regex_search(productHtml, match, pricePattern)) {
                        string priceText = (match.size() > 1 && !match[1].str().empty()) ? 
                                         match[1].str() : match[0].str();
                        product.price = cleanText(priceText);
                        if (!product.price.empty()) {
                            break;
                        }
                    }
                }
                
                // Extract rating
                for (const auto& ratingPattern : ratingPatterns) {
                    smatch match;
                    if (regex_search(productHtml, match, ratingPattern)) {
                        string ratingText = match[1];
                        if (!ratingText.empty()) {
                            product.rating = cleanText(ratingText);
                            break;
                        }
                    }
                }
                
                // Extract URL
                regex urlPattern("<a[^>]*href=\"([^\"]*)\"", regex_constants::icase);
                smatch urlMatch;
                if (regex_search(productHtml, urlMatch, urlPattern)) {
                    product.url = urlMatch[1];
                }
                
                // Only add products with meaningful data
                if (!product.name.empty() && (!product.price.empty() || !product.rating.empty())) {
                    products.push_back(product);
                    foundWithThisPattern++;
                }
            }
            
            cout << "Pattern " << (patternIndex + 1) << " found " << foundWithThisPattern << " products." << endl;
            
            // If we found a good number of products, we can break
            if (products.size() >= 10) {
                break;
            }
        }
        
        return products;
    }

    // Generate sample data for demonstration
    vector<Product> createSampleData(int count = 20) {
        vector<Product> products;
        
        for (int i = 0; i < min(count, static_cast<int>(sampleNames.size())); i++) {
            Product product;
            product.name = sampleNames[i];
            product.price = samplePrices[i];
            
            // Generate random rating between 3.0 and 5.0
            double rating = 3.0 + (rand() % 21) / 10.0;
            ostringstream ratingStream;
            ratingStream << fixed << setprecision(1) << rating;
            product.rating = ratingStream.str();
            
            product.url = "https://example-store.com/product/" + to_string(i + 1);
            
            products.push_back(product);
        }
        
        return products;
    }

    // Load HTML from file
    string loadHTMLFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return "";
        }
        
        string html, line;
        while (getline(file, line)) {
            html += line + "\n";
        }
        file.close();
        
        cout << "Loaded " << html.length() << " characters from " << filename << endl;
        return html;
    }

    // Save products to CSV
    void saveToCSV(const vector<Product>& products, const string& filename) {
        ofstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Error: Could not create CSV file: " << filename << endl;
            return;
        }
        
        // Write CSV header
        file << "Product Name,Price,Rating,URL\n";
        
        // Write product data
        for (const auto& product : products) {
            file << escapeCSV(product.name) << ","
                 << escapeCSV(product.price) << ","
                 << escapeCSV(product.rating) << ","
                 << escapeCSV(product.url) << "\n";
        }
        
        file.close();
        cout << "\n✓ Successfully saved " << products.size() << " products to " << filename << endl;
    }

    // Save products to JSON format as well
    void saveToJSON(const vector<Product>& products, const string& filename) {
        ofstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Error: Could not create JSON file: " << filename << endl;
            return;
        }
        
        file << "{\n  \"products\": [\n";
        
        for (size_t i = 0; i < products.size(); i++) {
            file << "    {\n";
            file << "      \"name\": \"" << products[i].name << "\",\n";
            file << "      \"price\": \"" << products[i].price << "\",\n";
            file << "      \"rating\": \"" << products[i].rating << "\",\n";
            file << "      \"url\": \"" << products[i].url << "\"\n";
            file << "    }";
            if (i < products.size() - 1) file << ",";
            file << "\n";
        }
        
        file << "  ]\n}\n";
        file.close();
        
        cout << "✓ Successfully saved " << products.size() << " products to " << filename << endl;
    }

    // Main processing function
    void processData(int choice, const string& input, const string& outputFile) {
        vector<Product> products;
        
        switch (choice) {
            case 1: {
                // Parse HTML file
                string html = loadHTMLFromFile(input);
                if (html.empty()) return;
                
                products = extractProducts(html);
                if (products.empty()) {
                    cout << "No products found in HTML file. The file might not contain recognizable e-commerce patterns." << endl;
                    cout << "Generating sample data instead..." << endl;
                    products = createSampleData(10);
                }
                break;
            }
            case 2: {
                // Generate sample data
                int count = 20;
                if (!input.empty()) {
                    count = stoi(input);
                }
                products = createSampleData(count);
                cout << "Generated " << products.size() << " sample products." << endl;
                break;
            }
            default: {
                cout << "Invalid choice." << endl;
                return;
            }
        }
        
        if (!products.empty()) {
            // Save to CSV
            saveToCSV(products, outputFile);
            
            // Also save to JSON
            string jsonFile = outputFile;
            size_t dotPos = jsonFile.find_last_of('.');
            if (dotPos != string::npos) {
                jsonFile = jsonFile.substr(0, dotPos) + ".json";
            } else {
                jsonFile += ".json";
            }
            saveToJSON(products, jsonFile);
            
            // Display sample products
            cout << "\n" << string(80, '=') << endl;
            cout << "EXTRACTED PRODUCTS PREVIEW:" << endl;
            cout << string(80, '=') << endl;
            
            for (size_t i = 0; i < min(products.size(), size_t(10)); ++i) {
                cout << "\nProduct #" << (i + 1) << ":" << endl;
                cout << "  Name: " << products[i].name << endl;
                cout << "  Price: " << products[i].price << endl;
                cout << "  Rating: " << products[i].rating << endl;
                cout << "  URL: " << products[i].url << endl;
            }
            
            if (products.size() > 10) {
                cout << "\n... and " << (products.size() - 10) << " more products." << endl;
            }
        }
    }

    // Create sample HTML file for testing
    void createSampleHTMLFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not create sample HTML file." << endl;
            return;
        }
        
        file << R"raw(<!DOCTYPE html>
<html>
<head>
    <title>Sample E-commerce Page</title>
</head>
<body>
    <div class="product-list">
        <div class="product-item">
            <h2 class="product-title">iPhone 14 Pro Max 128GB</h2>
            <span class="price">$1099.00</span>
            <div class="rating">4.5</div>
            <a href="/iphone-14-pro">View Product</a>
        </div>
        
        <div class="product-item">
            <h2 class="product-title">Samsung Galaxy S23 Ultra</h2>
            <span class="price">$1199.99</span>
            <div class="rating">4.4</div>
            <a href="/galaxy-s23">View Product</a>
        </div>
        
        <div class="product-item">
            <h2 class="product-title">MacBook Air M2 13-inch</h2>
            <span class="price">$1199.00</span>
            <div class="rating">4.7</div>
            <a href="/macbook-air">View Product</a>
        </div>
        
        <div class="product-item">
            <h2 class="product-title">Sony WH-1000XM4 Headphones</h2>
            <span class="price">$349.99</span>
            <div class="rating">4.6</div>
            <a href="/sony-headphones">View Product</a>
        </div>
        
        <div class="product-item">
            <h2 class="product-title">Dell XPS 13 Laptop</h2>
            <span class="price">$999.99</span>
            <div class="rating">4.3</div>
            <a href="/dell-xps13">View Product</a>
        </div>
    </div>
</body>
</html>)raw";
        
        file.close();
        cout << "✓ Sample HTML file created: " << filename << endl;
    }
};

int main() {
    EcommerceScraper scraper;
    
    cout << string(60, '=') << endl;
    cout << "       E-COMMERCE PRODUCT SCRAPER" << endl;
    cout << string(60, '=') << endl;
    cout << "Options:" << endl;
    cout << "1. Parse HTML file (recommended)" << endl;
    cout << "2. Generate sample data" << endl;
    cout << "3. Create sample HTML file for testing" << endl;
    cout << string(60, '-') << endl;
    cout << "Enter your choice (1-3): ";
    
    int choice;
    cin >> choice;
    cin.ignore(); // Clear input buffer
    
    if (choice == 3) {
        string htmlFile;
        cout << "Enter filename for sample HTML (e.g., sample.html): ";
        getline(cin, htmlFile);
        if (htmlFile.empty()) htmlFile = "sample.html";
        
        scraper.createSampleHTMLFile(htmlFile);
        cout << "You can now use option 1 to parse this file!" << endl;
        cout << "Press Enter to exit...";
        cin.get();
        return 0;
    }
    
    string input, outputFile;
    
    switch (choice) {
        case 1: {
            cout << "Enter HTML file path: ";
            getline(cin, input);
            if (input.empty()) {
                cout << "No file specified. Creating and using sample HTML file..." << endl;
                input = "sample.html";
                scraper.createSampleHTMLFile(input);
            }
            break;
        }
        case 2: {
            cout << "Enter number of sample products to generate (default 20): ";
            getline(cin, input);
            if (input.empty()) input = "20";
            break;
        }
        default: {
            cout << "Invalid choice. Generating sample data..." << endl;
            choice = 2;
            input = "20";
            break;
        }
    }
    
    cout << "Enter output CSV filename (default: products.csv): ";
    getline(cin, outputFile);
    
    if (outputFile.empty()) {
        outputFile = "products.csv";
    }
    
    // Add .csv extension if not present
    if (outputFile.find(".csv") == string::npos) {
        outputFile += ".csv";
    }
    
    try {
        scraper.processData(choice, input, outputFile);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    
    cout << "\n" << string(60, '=') << endl;
    cout << "Program completed successfully!" << endl;
    cout << "Press Enter to exit...";
    cin.get();
    
    return 0;
}