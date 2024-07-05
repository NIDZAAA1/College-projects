
package student;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import rs.etf.sab.operations.CityOperations;

/**
 *
 * @author nikol
 */
public class sn210229_CityOperations implements CityOperations {
    private static Connection conn = null;
    private static sn210229_CityOperations inst = null;
    
    private sn210229_CityOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_CityOperations getInstance(){
        if (inst == null) inst = new sn210229_CityOperations();
        return inst;
    }
    
    
    public int deleteCity(String name){ 
        String query = "select Id from Grad where Naziv = ?";
        int rez = 0;
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,name);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                deleteCity(rs.getInt(1));
                rez+=1;
            }
            return rez;
        } catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    
    @Override
    public int insertCity(String name, String postalCode) { 
        if(postalCodeExists(postalCode))return -1;
        if(cityExists(name))return -1;
        String query = "insert into Grad (Naziv,PostanskiBroj)" +
                " values (?,?)";
        try {
            PreparedStatement ps = conn.prepareStatement(query, PreparedStatement.RETURN_GENERATED_KEYS);
            ps.setString(1, name);
            ps.setString(2, postalCode);
            ps.executeUpdate();
            ResultSet rs = ps.getGeneratedKeys();
            if(rs.next()){
                return rs.getInt(1);
            }
            return -1;
        } catch (SQLException ex) {
            ex.printStackTrace();
            return -1;
        }
    }

    @Override
    public int deleteCity(String... strings) { 
        int rez = 0;
        for (String s : strings) {
            rez+= deleteCity(s);
        };
        return rez;
    }

    @Override
    public boolean deleteCity(int idGrad) { 
        if(getAllCities().contains(idGrad)==false)return false;
        String query = "delete from Grad where Id = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setInt(1,idGrad);
            int rez =  ps.executeUpdate();
            if(rez!=0)return true;
            return false;
        } catch (Exception e) {e.printStackTrace();return false;}
    }

    @Override
    public List<Integer> getAllCities() { 
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select Id from Grad";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ResultSet rs = ps.executeQuery();
            while(rs.next()){
                rez.add(rs.getInt(1));
            }

        }catch (Exception e) {e.printStackTrace();}
        return rez;
    }
    
    public boolean cityExists(String cityName){
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select Id from Grad where Naziv = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,cityName);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                return true;
            }

        }catch (Exception e) {e.printStackTrace();return false;}
        return false;
    }
    public boolean postalCodeExists(String postalCode){
        ArrayList<Integer> rez = new ArrayList<>();
        String query = "select Id from Grad where PostanskiBroj = ?";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.setString(1,postalCode);
            ResultSet rs = ps.executeQuery();
            if(rs.next()){
                return true;
            }

        }catch (Exception e) {e.printStackTrace();return false;}
        return false;
    }
}
