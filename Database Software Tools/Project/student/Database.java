package student;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Database {
    private static Database instance;
    private Connection connection;
    private static final String USER = "sa";
    private static final String PASS = "123";
    private static final String DB_NAME = "Transport";
    private static final int PORT = 1433;
    private static final String SERVER = "localhost";
    private static final String URL = "jdbc:sqlserver:
    
    private Database() {
        try {
            this.connection = DriverManager.getConnection(URL, USER, PASS);
        } catch (SQLException e) {
           e.printStackTrace();
        }
    }
    
    public Connection getConnection() {
        return connection;
    }
    
    public static synchronized Database getInstance() {
        if (instance == null) {
            instance = new Database();
        }
        return instance;
    }
}
