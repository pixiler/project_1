----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/25/2021 04:17:35 PM
-- Design Name: 
-- Module Name: test_top - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity test_top is
    Port (
    clk : in std_logic;
    LED : out std_logic_vector(1 downto 0)
    );
end test_top;

architecture Behavioral of test_top is

    signal s_LED : std_logic_vector(1 downto 0) := (others => '0');

begin

    P_MAIN : process(clk) begin
        if(rising_edge(clk)) then
            s_LED <= not s_LED;
        end if;
    end process P_MAIN;

    LED <= s_LED;

end Behavioral;
